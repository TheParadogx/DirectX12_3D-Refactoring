#pragma once
#include<Utility/Types/EcseTypes.hpp>

namespace Ecse::Graphics
{

	/// <summary>
	/// Sprite用のパイプライン（インスタンシングは処理効率が気になったら取り入れます。現状は必要性が低いかも）
	/// </summary>
	class SpritePipeline
	{
		/// <summary>
		/// 深度ステンシルの初期設定の取得
		/// </summary>
		/// <returns></returns>
		D3D12_DEPTH_STENCIL_DESC GetDepthStencilDesc();

		/// <summary>
		/// ブレンドステートの初期設定の取得
		/// </summary>
		/// <returns></returns>
		D3D12_BLEND_DESC GetBlendStateDesc();

		/// <summary>
		/// ラスタライザーの初期設定の取得
		/// </summary>
		/// <returns></returns>
		D3D12_RASTERIZER_DESC GetRasterizerDesc();

		/// <summary>
		/// ルートシグネチャの作成
		/// </summary>
		/// <returns>true:成功</returns>
		bool CreateRootSignature();

		/// <summary>
		/// 描画パイプラインの作成
		/// </summary>
		/// <returns></returns>
		bool CreatePipeline();
	public:
		SpritePipeline();
		virtual ~SpritePipeline() = default;

		/// <summary>
		/// 初期化
		/// </summary>
		/// <returns></returns>
		bool Create();

		/// <summary>
		/// ルートシグネチャの取得
		/// </summary>
		/// <returns></returns>
		ID3D12RootSignature* GetRootSignature()const;

		/// <summary>
		/// パイプラインステートの取得
		/// </summary>
		/// <returns></returns>
		ID3D12PipelineState* GetPipelineState()const;
	private:
		/// <summary>
		/// ルートシグネチャ
		/// </summary>
		RootSig mRootSignature;
		/// <summary>
		/// パイプラインステートオブジェクト
		/// </summary>
		PSO mPipelineState;
	};
}

