#pragma once
#include<Utility/Types/EcseTypes.hpp>

namespace Ecse::Graphics
{
	/// <summary>
	/// Fbx用のパイプライン
	/// </summary>
	class FbxPipeline
	{
	public:
		FbxPipeline();
		virtual ~FbxPipeline() = default;

		/// <summary>
		/// 初期化
		/// </summary>
		/// <returns>true:成功</returns>
		bool Create();

		// アクセサ
		ID3D12RootSignature* GetRootSignature() const;
		ID3D12PipelineState* GetPipelineState() const;
	private:
		/// <summary>
		/// 3D描画用の深度設定（Zテスト・Zライト有効）
		/// </summary>
		D3D12_DEPTH_STENCIL_DESC GetDepthStencilDesc();

		/// <summary>
		/// 3D描画用のブレンド設定（基本は不透明、必要に応じてアルファテスト）
		/// </summary>
		D3D12_BLEND_DESC GetBlendStateDesc();

		/// <summary>
		/// 3D描画用のラスタライザー設定（背面カリング有効）
		/// </summary>
		D3D12_RASTERIZER_DESC GetRasterizerDesc();

		bool CreateRootSignature();
		bool CreatePipeline();
	private:

		RootSig mRootSignature;
		PSO mPipelineState;
	};
}


