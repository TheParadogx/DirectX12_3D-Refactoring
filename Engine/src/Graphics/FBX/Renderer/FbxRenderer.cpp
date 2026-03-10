#include "pch.h"
#include "FbxRenderer.hpp"
#include<Graphics/FBX/Resource/FbxResource.hpp>
#include<Graphics/DX12/DX12.hpp>
#include<ECS/Component/Transform/TransformComponent.hpp>
#include<ECS/Component/FBX/FbxComponent.hpp>

using namespace DirectX;

namespace Ecse::Graphics
{
	bool FbxRenderer::Initialize()
	{
		auto device = SYS::ServiceLocator::Get<DX12>()->GetDevice();
		auto frameCount = SYS::ServiceLocator::Get<DX12>()->FRAME_COUNT; // フレームカウントは0始まりなので+1して実際の数を取得

		// 定数バッファのアロケーターの初期化
		mConstantBuffer = std::make_unique<DynamicBuffer>(device, 2 * 1024 * 1024, frameCount);

		// パイプライン
		mPipeline = std::make_unique<FbxPipeline>();
		if (!mPipeline->Create())
		{
			ECSE_LOG(SYS::eLogLevel::Error, "Failed to create FbxPipeline.");
			return false;
		}

		return true;
	}
	/// <summary>
	/// 初期化
	/// </summary>
	/// <returns></returns>
	bool FbxRenderer::Initialize(ID3D12Device* device, uint32_t frameCount)
	{
		// 定数バッファのアロケーターの初期化
		mConstantBuffer = std::make_unique<DynamicBuffer>(device, 2 * 1024 * 1024, frameCount);

		// パイプライン
		mPipeline = std::make_unique<FbxPipeline>();
		if(!mPipeline->Create())
		{
			ECSE_LOG(SYS::eLogLevel::Error, "Failed to create FbxPipeline.");
			return false;
		}

		return true;
	}

	/// <summary>
	/// 表示
	/// </summary>
	void FbxRenderer::Render(entt::registry& registry, ID3D12GraphicsCommandList* cmdList, uint32_t frameIndex, const XMMATRIX& viewProj)
	{
		// フレームの先頭で現在のフレーム用オフセットをリセット
		mConstantBuffer->Reset(frameIndex);

		// パイプラインバインド
		cmdList->SetGraphicsRootSignature(mPipeline->GetRootSignature());
		cmdList->SetPipelineState(mPipeline->GetPipelineState());
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// SceneConstant (b3) の転送
		SceneConstant scene;
		XMStoreFloat4x4(&scene.viewProjection, XMMatrixTranspose(viewProj));
		// 型安全な Upload<T> を使用
		cmdList->SetGraphicsRootConstantBufferView(0, mConstantBuffer->Upload(frameIndex, scene));

		auto view = registry.view<ECS::Transform3D, ECS::FbxComponent>();

		view.each([&](auto entity, ECS::Transform3D& transform, ECS::FbxComponent& fbx)
			{
				// リソースの有効判定
				if (!fbx.Resource) return;

				// MeshConstant (b4) の転送
				MeshConstant mesh;
				XMStoreFloat4x4(&mesh.world, XMMatrixTranspose(transform.GetWorldMatrix()));
				cmdList->SetGraphicsRootConstantBufferView(1, mConstantBuffer->Upload(frameIndex, mesh));

				// BoneConstant (b5) の転送
				BoneConstant boneData{};

				std::vector<DirectX::XMFLOAT4X4> defaultBones;
				const auto& sourceBones = fbx.BoneTransforms.empty()
					? (defaultBones = fbx.Resource->GetDefaultBoneTransforms(), defaultBones)
					: fbx.BoneTransforms;

				uint32_t copyCount = (uint32_t)std::min<size_t>(sourceBones.size(), MAX_BONES);
				if (copyCount > 0) {
					std::memcpy(boneData.bones, sourceBones.data(), sizeof(XMFLOAT4X4) * copyCount);
				}

				// Upload<T> により、8192バイトが型安全かつ 256byteアラインメント済みで転送される
				cmdList->SetGraphicsRootConstantBufferView(2, mConstantBuffer->Upload(frameIndex, boneData));

				// モデルデータのバインド
				fbx.Resource->SetBuffers(cmdList);

				for (const auto& mat : fbx.Resource->GetMaterials()) {
					// b3: テクスチャバインド (RootDescriptorTable の 3番を想定)
					if (mat.Texture) {
						// Ecse::Graphics::Texture の実装に合わせて GPU ハンドルをセット
						// cmdList->SetGraphicsRootDescriptorTable(3, mat.Texture->GetGpuHandle());
					}

					// ドローコール
					cmdList->DrawIndexedInstanced(
						mat.IndexCount,
						1,
						mat.StartIndex,
						0,
						0
					);
				}
			});
	}
}
