#pragma once
#include <Utility/Export/Export.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/BodyInterface.h>

#include <DirectXMath.h>

namespace Ecse::Physics
{
    // ==================================================================
    //  Rigidbody3D
    //  Transform3D と同じエンティティに付ける。
    //  Jolt の Body を1つ対応させ、BodyID を保持する。
    // ==================================================================
    struct ENGINE_API Rigidbody3D
    {
        // ----------------------------------------------------------
        //  設定（Body 生成前に書く）
        // ----------------------------------------------------------

        /// 質量（kg）。isKinematic = true の場合は無視される
        float Mass = 1.0f;

        /// 線形減衰（空気抵抗など）。0.0 = 減衰なし
        float LinearDamping = 0.0f;

        /// 角度減衰
        float AngularDamping = 0.0f;

        /// true = 重力を受ける
        bool UseGravity = true;

        /// true = コード制御のみ（物理演算を受けない）
        /// Transform3D の変更が毎フレーム Jolt に反映される
        bool IsKinematic = false;

        // ----------------------------------------------------------
        //  力の累積（ユーザーが AddForce / AddImpulse で書き込む）
        //  RigidbodySystem の PreUpdate で Jolt に渡してリセット
        // ----------------------------------------------------------
        DirectX::XMFLOAT3 AccumulatedForce = { 0, 0, 0 };
        DirectX::XMFLOAT3 AccumulatedImpulse = { 0, 0, 0 };

        // ----------------------------------------------------------
        //  Jolt の BodyID（RigidbodySystem が書き込む・ユーザー変更不可）
        // ----------------------------------------------------------
        JPH::BodyID BodyId = JPH::BodyID();

        // ----------------------------------------------------------
        //  ヘルパー：力・衝撃を累積する
        //  ユーザーシステムの Update 内から呼ぶ
        // ----------------------------------------------------------
        void AddForce(const DirectX::XMFLOAT3& force)
        {
            AccumulatedForce.x += force.x;
            AccumulatedForce.y += force.y;
            AccumulatedForce.z += force.z;
        }

        void AddImpulse(const DirectX::XMFLOAT3& impulse)
        {
            AccumulatedImpulse.x += impulse.x;
            AccumulatedImpulse.y += impulse.y;
            AccumulatedImpulse.z += impulse.z;
        }

        void ResetAccumulated()
        {
            AccumulatedForce = { 0, 0, 0 };
            AccumulatedImpulse = { 0, 0, 0 };
        }
    };

    // ==================================================================
    //  Rigidbody2D
    //  Transform2D と同じエンティティに付ける。
    //  内部では Jolt の3D Body を使い、Z軸移動と X/Y 回転を拘束する。
    // ==================================================================
    struct ENGINE_API Rigidbody2D
    {
        float Mass = 1.0f;
        float LinearDamping = 0.0f;
        float AngularDamping = 0.0f;
        bool  UseGravity = true;
        bool  IsKinematic = false;

        // 2D なので力・衝撃も XY 平面のみ
        DirectX::XMFLOAT2 AccumulatedForce = { 0, 0 };
        DirectX::XMFLOAT2 AccumulatedImpulse = { 0, 0 };

        JPH::BodyID BodyId = JPH::BodyID();

        void AddForce(const DirectX::XMFLOAT2& force)
        {
            AccumulatedForce.x += force.x;
            AccumulatedForce.y += force.y;
        }

        void AddImpulse(const DirectX::XMFLOAT2& impulse)
        {
            AccumulatedImpulse.x += impulse.x;
            AccumulatedImpulse.y += impulse.y;
        }

        void ResetAccumulated()
        {
            AccumulatedForce = { 0, 0 };
            AccumulatedImpulse = { 0, 0 };
        }
    };

} // namespace Ecse::Physics