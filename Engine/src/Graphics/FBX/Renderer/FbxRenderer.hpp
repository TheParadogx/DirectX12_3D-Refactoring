#pragma once
#include <d3d12.h>
#include <memory>
#include <vector>
#include <entt/entt.hpp>
#include<DirectXMath.h>
#include<Graphics/DynamicBuffer/DynamicBuffer.hpp>
#include<Graphics/FBX/Pipeline/FbxPipeline.hpp>


namespace Ecse::Graphics
{
	class FbxPipeline;
	class DynamicBuffer;

	/// <summary>
	/// FBXモデルのレンダリング管理
	/// </summary>
	class FbxRenderer
	{
	public:
		// 定数バッファの構造体定義
		struct SceneConstant {
			DirectX::XMFLOAT4X4 viewProjection;
		};

		struct MeshConstant {
			DirectX::XMFLOAT4X4 world;
		};

		static constexpr uint32_t MAX_BONES = 256;
		struct BoneConstant {
			DirectX::XMFLOAT4X4 bones[MAX_BONES];
		};

		FbxRenderer() = default;
		virtual ~FbxRenderer() = default;

		/// <summary>
		/// 初期化
		/// </summary>
		/// <returns></returns>
		bool Initialize();
		bool Initialize(ID3D12Device* device, uint32_t frameCount);

		/// <summary>
		/// 表示
		/// </summary>
		void Render(entt::registry& registry, ID3D12GraphicsCommandList* cmdList, uint32_t frameIndex, const DirectX::XMMATRIX& viewProj);

	private:
		// パイプライン状態の管理
		std::unique_ptr<FbxPipeline> mPipeline;

		// 動的定数バッファの管理
		std::unique_ptr<DynamicBuffer> mConstantBuffer;
	};
}

