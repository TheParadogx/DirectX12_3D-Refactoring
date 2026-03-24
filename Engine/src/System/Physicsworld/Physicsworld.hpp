#pragma once
#include <Utility/Singleton/Singleton.hpp>
#include <Utility/Export/Export.hpp>

// Jolt
#include <Jolt/Jolt.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemSingleThreaded.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>


namespace Ecse::Physics
{
    // ------------------------------------------------------------------
    //  ObjectLayer
    //  衝突レイヤーの定義
    //  Static  : 床・壁など動かない物体
    //  Dynamic : 物理演算で動く物体
    //  Sensor  : 押し返しなし・重なり検出のみ（Trigger）
    // ------------------------------------------------------------------
    namespace ObjectLayer
    {
        constexpr JPH::ObjectLayer Static = 0;
        constexpr JPH::ObjectLayer Dynamic = 1;
        constexpr JPH::ObjectLayer Sensor = 2;
        constexpr JPH::ObjectLayer Count = 3;
    }

    namespace BroadPhaseLayer
    {
        constexpr JPH::BroadPhaseLayer NonMoving = JPH::BroadPhaseLayer(0);
        constexpr JPH::BroadPhaseLayer Moving = JPH::BroadPhaseLayer(1);
        constexpr JPH::uint Count = 2;
    }

    // ------------------------------------------------------------------
    //  PhysicsWorld
    //
    //  Jolt の初期化・破棄・Step を一手に担うシングルトン。
    //  RigidbodySystem からのみアクセスする。
    //
    //  使い方:
    //    初期化: PhysicsWorld::GetInstance().Initialize();
    //    毎フレーム: PhysicsWorld::GetInstance().Step(dt);
    //    終了: PhysicsWorld::GetInstance().Shutdown();
    // ------------------------------------------------------------------
    class ENGINE_API PhysicsWorld : public Utility::Singleton<PhysicsWorld>
    {
        SINGLETON_CLASS(PhysicsWorld);
    public:
        SINGLETON_ACCESSOR(PhysicsWorld);

        // ----------------------------------------------------------
        //  Initialize / Shutdown
        // ----------------------------------------------------------
        bool Initialize(
            uint32_t maxBodies = 1024,
            uint32_t maxBodyPairs = 1024,
            uint32_t maxContactConstraints = 1024);

        void Shutdown();

        // ----------------------------------------------------------
        //  Step
        //  deltaTime  : フレーム時間（秒）
        //  subSteps   : 1フレームを何分割してシミュする か（精度向上）
        // ----------------------------------------------------------
        void Step(float deltaTime, int subSteps = 1);

        // ----------------------------------------------------------
        //  BodyInterface へのアクセス
        //  Rigidbody コンポーネントがボディの追加・削除・操作に使う
        // ----------------------------------------------------------
        [[nodiscard]] JPH::BodyInterface& GetBodyInterface();

        // ----------------------------------------------------------
        //  重力の設定・取得
        // ----------------------------------------------------------
        void SetGravity(const JPH::Vec3& gravity);
        [[nodiscard]] JPH::Vec3 GetGravity() const;

        // ----------------------------------------------------------
        //  BodyID の発行
        //  Collider の形状を受け取り、Body を生成して BodyID を返す
        // ----------------------------------------------------------
        [[nodiscard]] JPH::BodyID CreateBody(
            const JPH::BodyCreationSettings& settings);

        // ----------------------------------------------------------
        //  BodyID の破棄
        // ----------------------------------------------------------
        void DestroyBody(JPH::BodyID bodyId);

    private:
        // Jolt が要求するコールバック実装（内部クラス）
        class ObjectLayerPairFilterImpl;
        class BroadPhaseLayerInterfaceImpl;
        class ObjectVsBroadPhaseLayerFilterImpl;

    private:
        static constexpr uint32_t kTempAllocatorSize = 10 * 1024 * 1024; // 10MB

        std::unique_ptr<JPH::TempAllocatorImpl>            mTempAllocator;
        std::unique_ptr<JPH::JobSystemSingleThreaded>      mJobSystem;
        std::unique_ptr<BroadPhaseLayerInterfaceImpl>      mBroadPhaseLayerInterface;
        std::unique_ptr<ObjectVsBroadPhaseLayerFilterImpl> mObjectVsBroadPhaseFilter;
        std::unique_ptr<ObjectLayerPairFilterImpl>         mObjectLayerPairFilter;
        std::unique_ptr<JPH::PhysicsSystem>                mPhysicsSystem;

        bool mInitialized = false;
    };

} // namespace Ecse::Physics