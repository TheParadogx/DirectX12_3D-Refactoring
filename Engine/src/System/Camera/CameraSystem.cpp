#include "pch.h"
#include<System/Camera/CameraSystem.hpp>
#include<ECS/Component/Camera/CameraComponent.hpp>
#include<ECS/Component/Transform/TransformComponent.hpp>

namespace Ecse::System
{
	/// <summary>
	/// 全てのカメラエンティティの行列を更新する
	/// Viewは毎フレーム、Projectionは変更フラグがある時のみ計算
	/// </summary>
	void CameraSystem::Update(entt::registry& Registry)
	{

		using namespace DirectX;

		auto view = Registry.view<ECS::CameraComponent, ECS::Transform3D>();
		view.each([](ECS::CameraComponent& camera, ECS::Transform3D& trans) 
			{
				// Viewは毎フレーム更新
				camera.ViewMatrix = XMMatrixLookToLH(
					XMLoadFloat3(&trans.Position),
					XMVector3Normalize(trans.GetForward()),
					XMVector3Normalize(trans.GetUp())
				);

				// Projectionは変更フラグがあるときのみ更新
				if (camera.NeedsUpdateProjection) 
				{
					camera.ProjectionMatrix = XMMatrixPerspectiveFovLH(
						camera.FovAngleY, camera.AspectRatio, camera.NearZ, camera.FarZ
					);
					camera.NeedsUpdateProjection = false;
				}
			});
	}

	/// <summary>
	/// メインカメラに指定されているエンティティからViewとProjection行列を取得する
	/// </summary>
	/// <returns>有効なメインカメラが存在すればtrue</returns>
	bool CameraSystem::GetMainCameraMatrices(entt::registry& Registry, DirectX::XMMATRIX& OutView, DirectX::XMMATRIX& OutProj)
	{
		const auto view = Registry.view<ECS::MainCameraTag, ECS::CameraComponent>();
		if (view.size_hint() <= 0)
		{
			return false;
		}

		const auto entity = view.front();
		const ECS::CameraComponent& camera = view.get<ECS::CameraComponent>(entity);
		OutView = camera.ViewMatrix;
		OutProj = camera.ProjectionMatrix;

		return true;
	}

	/// <summary>
	/// メインカメラのViewとProjectionを掛け合わせた行列を取得する
	/// </summary>
	/// <param name="registry"></param>
	/// <returns></returns>
	DirectX::XMMATRIX CameraSystem::GetMainCameraViewProj(entt::registry& registry)
	{
		DirectX::XMMATRIX view, proj;

		if (!GetMainCameraMatrices(registry, view, proj))
		{
			return DirectX::XMMatrixIdentity();
		}

		return view * proj;
	}
}