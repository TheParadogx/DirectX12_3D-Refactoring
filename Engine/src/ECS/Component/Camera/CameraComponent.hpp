#pragma once
#include<DirectXMath.h>

namespace Ecse::ECS
{
	/// <summary>
	/// カメラコンポーネント
	/// </summary>
	struct CameraComponent
	{
		float FovY = DirectX::XMConvertToRadians(45.0f);	//	視野角
		float AspectRatio = 16 / 9; // アスペクト比
		float NearZ = 0.1f; 
		float FarZ = 1000.0f;

		//　ワールド行列はTransformComponentで管理するため、ビュー行列とプロジェクション行列のみを保持
		DirectX::XMMATRIX ViewMatrix = DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX ProjectionMatrix = DirectX::XMMatrixIdentity();

		CameraComponent() = default;
	};
}


/// <summary>
/// メインカメラのタグコンポーネント
/// </summary>
struct MainCameraTag {};