#include "pch.h"
#include"Physicsworld.hpp"

// Jolt
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

namespace Ecse::Physics
{
    // ------------------------------------------------------------------
    //  ObjectLayer 同士が衝突するか
    // ------------------------------------------------------------------
    class PhysicsWorld::ObjectLayerPairFilterImpl
        : public JPH::ObjectLayerPairFilter
    {
    public:
        bool ShouldCollide(
            JPH::ObjectLayer a,
            JPH::ObjectLayer b) const override
        {
            switch (a)
            {
            case ObjectLayer::Static:
                // Static は Dynamic とだけ衝突（Static同士・Sensor は無視）
                return b == ObjectLayer::Dynamic;

            case ObjectLayer::Dynamic:
                // Dynamic は Static・Dynamic と衝突（Sensor は無視）
                return b == ObjectLayer::Static
                    || b == ObjectLayer::Dynamic;

            case ObjectLayer::Sensor:
                // Sensor は衝突反応なし（検出は ContactListener で行う）
                return false;

            default:
                return false;
            }
        }
    };

    // ------------------------------------------------------------------
    //  BroadPhase レイヤーのマッピング
    // ------------------------------------------------------------------
    class PhysicsWorld::BroadPhaseLayerInterfaceImpl
        : public JPH::BroadPhaseLayerInterface
    {
    public:
        BroadPhaseLayerInterfaceImpl()
        {
            mObjectToBroadPhase[ObjectLayer::Static] = BroadPhaseLayer::NonMoving;
            mObjectToBroadPhase[ObjectLayer::Dynamic] = BroadPhaseLayer::Moving;
            mObjectToBroadPhase[ObjectLayer::Sensor] = BroadPhaseLayer::Moving;
        }

        JPH::uint GetNumBroadPhaseLayers() const override
        {
            return BroadPhaseLayer::Count;
        }

        JPH::BroadPhaseLayer GetBroadPhaseLayer(
            JPH::ObjectLayer layer) const override
        {
            return mObjectToBroadPhase[layer];
        }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        const char* GetBroadPhaseLayerName(
            JPH::BroadPhaseLayer layer) const override
        {
            return layer == BroadPhaseLayer::NonMoving ? "NonMoving" : "Moving";
        }
#endif

    private:
        JPH::BroadPhaseLayer mObjectToBroadPhase[ObjectLayer::Count];
    };

    // ------------------------------------------------------------------
    //  Object レイヤー vs BroadPhase レイヤー のフィルタ
    // ------------------------------------------------------------------
    class PhysicsWorld::ObjectVsBroadPhaseLayerFilterImpl
        : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        bool ShouldCollide(
            JPH::ObjectLayer     objectLayer,
            JPH::BroadPhaseLayer broadPhaseLayer) const override
        {
            switch (objectLayer)
            {
            case ObjectLayer::Static:
                return broadPhaseLayer == BroadPhaseLayer::Moving;
            case ObjectLayer::Dynamic:
                return true;
            case ObjectLayer::Sensor:
                return broadPhaseLayer == BroadPhaseLayer::Moving;
            default:
                return false;
            }
        }
    };

    // ------------------------------------------------------------------
    //  Initialize
    // ------------------------------------------------------------------
    bool PhysicsWorld::Initialize(
        uint32_t maxBodies,
        uint32_t maxBodyPairs,
        uint32_t maxContactConstraints)
    {
        if (mInitialized) return true;

        // Jolt のグローバル初期化（1度だけ呼ぶ）
        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();

        // アロケータ・ジョブシステム
        mTempAllocator = std::make_unique<JPH::TempAllocatorImpl>(
            kTempAllocatorSize);
        mJobSystem = std::make_unique<JPH::JobSystemSingleThreaded>(
            JPH::cMaxPhysicsJobs);

        // レイヤーフィルタ
        mBroadPhaseLayerInterface = std::make_unique<BroadPhaseLayerInterfaceImpl>();
        mObjectVsBroadPhaseFilter = std::make_unique<ObjectVsBroadPhaseLayerFilterImpl>();
        mObjectLayerPairFilter = std::make_unique<ObjectLayerPairFilterImpl>();

        // PhysicsSystem 本体
        mPhysicsSystem = std::make_unique<JPH::PhysicsSystem>();
        mPhysicsSystem->Init(
            maxBodies,
            0, // inNumBodyMutexes (0 = auto)
            maxBodyPairs,
            maxContactConstraints,
            *mBroadPhaseLayerInterface,
            *mObjectVsBroadPhaseFilter,
            *mObjectLayerPairFilter);

        // デフォルト重力（Y下向き）
        mPhysicsSystem->SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));

        mInitialized = true;
        return true;
    }

    // ------------------------------------------------------------------
    //  Shutdown
    // ------------------------------------------------------------------
    void PhysicsWorld::Shutdown()
    {
        if (!mInitialized) return;

        mPhysicsSystem.reset();
        mObjectLayerPairFilter.reset();
        mObjectVsBroadPhaseFilter.reset();
        mBroadPhaseLayerInterface.reset();
        mJobSystem.reset();
        mTempAllocator.reset();

        // Jolt グローバル解放
        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;

        mInitialized = false;
    }

    // ------------------------------------------------------------------
    //  Step
    // ------------------------------------------------------------------
    void PhysicsWorld::Step(float deltaTime, int subSteps)
    {
        if (!mInitialized) return;
        mPhysicsSystem->Update(
            deltaTime,
            subSteps,
            mTempAllocator.get(),
            mJobSystem.get());
    }

    // ------------------------------------------------------------------
    //  BodyInterface
    // ------------------------------------------------------------------
    JPH::BodyInterface& PhysicsWorld::GetBodyInterface()
    {
        return mPhysicsSystem->GetBodyInterface();
    }

    // ------------------------------------------------------------------
    //  Gravity
    // ------------------------------------------------------------------
    void PhysicsWorld::SetGravity(const JPH::Vec3& gravity)
    {
        mPhysicsSystem->SetGravity(gravity);
    }

    JPH::Vec3 PhysicsWorld::GetGravity() const
    {
        return mPhysicsSystem->GetGravity();
    }

    // ------------------------------------------------------------------
    //  CreateBody
    // ------------------------------------------------------------------
    JPH::BodyID PhysicsWorld::CreateBody(
        const JPH::BodyCreationSettings& settings)
    {
        auto& bi = mPhysicsSystem->GetBodyInterface();
        auto* body = bi.CreateBody(settings);
        if (!body) return JPH::BodyID(); // 生成失敗（上限超過など）

        bi.AddBody(body->GetID(), JPH::EActivation::Activate);
        return body->GetID();
    }

    // ------------------------------------------------------------------
    //  DestroyBody
    // ------------------------------------------------------------------
    void PhysicsWorld::DestroyBody(JPH::BodyID bodyId)
    {
        if (bodyId.IsInvalid()) return;
        auto& bi = mPhysicsSystem->GetBodyInterface();
        bi.RemoveBody(bodyId);
        bi.DestroyBody(bodyId);
    }

} // namespace Ecse::Physics