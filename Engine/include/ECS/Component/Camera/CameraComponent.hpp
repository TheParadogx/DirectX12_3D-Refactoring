#pragma once
#include <DirectXMath.h>

namespace Ecse::ECS
{
    /// <summary>
    /// カメラコンポーネント
    /// このコンポーネントを持つオブジェクトをカメラとする。
    /// </summary>
    struct CameraComponent {
        // 設定パラメータ
        float FovAngleY = DirectX::XMConvertToRadians(45.0f); // 視野角
        float AspectRatio = 16.0f / 9.0f;                     // アスペクト比
        float NearZ = 0.1f;                                   // 前方クリップ
        float FarZ = 1000.0f;                                 // 後方クリップ

        // 生成される行列
        DirectX::XMMATRIX ViewMatrix;
        DirectX::XMMATRIX ProjectionMatrix;

        // ビュー・プロジェクションを掛け合わせた行列
        DirectX::XMMATRIX GetViewProjection() const {
            return ViewMatrix * ProjectionMatrix;
        }
    };
}