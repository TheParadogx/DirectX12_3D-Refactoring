#pragma once
#include<Utility/Export/Export.hpp>

#include<DirectXMath.h>


/*
* ２Dと３Dは別のコンポーネントとして作ります。
*/

namespace Ecse::ECS
{
	/// <summary>
	/// 2D座標系用
	/// </summary>
	struct ENGINE_API Transform2D
	{
		// 座標
		DirectX::XMFLOAT2 Position = {0,0};
		// 全体倍率
		DirectX::XMFLOAT2 Scale = {1,1};
		// 回転
		float Rotation = 0.0f;
	};

    /// <summary>
	/// 3D座標系用
    /// </summary>
    struct ENGINE_API Transform3D {
        // 座標
        DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };
        // 回転
        DirectX::XMFLOAT4 Rotation = { 0.0f, 0.0f, 0.0f, 1.0f }; // Quaternion
		// 全体倍率
        DirectX::XMFLOAT3 Scale = { 1.0f, 1.0f, 1.0f };

        /// <summary>
        /// オブジェクトの前方ベクトルを取得 (Z+)
        /// </summary>
        inline DirectX::XMVECTOR GetForward() const {
            using namespace DirectX;
            XMVECTOR rot = XMLoadFloat4(&Rotation);
            // {} ではなく XMVectorSet を使用
            return XMVector3Rotate(XMVectorSet(0, 0, 1, 0), rot);
        }

        /// <summary>
        /// オブジェクトの上方向ベクトルを取得 (Y+)
        /// </summary>
        inline DirectX::XMVECTOR GetUp() const {
            using namespace DirectX;
            XMVECTOR rot = XMLoadFloat4(&Rotation);
            return XMVector3Rotate(XMVectorSet(0, 1, 0, 0), rot);
        }

        /// <summary>
        /// ワールド行列を生成
        /// </summary>
        inline DirectX::XMMATRIX GetWorldMatrix() const {
            using namespace DirectX;
            return XMMatrixAffineTransformation(
                XMLoadFloat3(&Scale),
                XMVectorSet(0, 0, 0, 1), // 原点
                XMLoadFloat4(&Rotation),
                XMLoadFloat3(&Position)
            );
        }

        void SetPosition(DirectX::FXMVECTOR pos) {
            DirectX::XMStoreFloat3(&Position, pos);
        }

        void SetRotation(DirectX::FXMVECTOR rot) {
            DirectX::XMStoreFloat4(&Rotation, rot);
        }
    };
}