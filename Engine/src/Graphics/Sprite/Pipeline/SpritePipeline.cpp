#include "pch.h"
#include "SpritePipeline.hpp"

#include<Graphics/DX12/DX12.hpp>
#include<Graphics/Shader/ShaderManager.hpp>


namespace Ecse::Graphics
{
	SpritePipeline::SpritePipeline()
		:mRootSignature(nullptr)
		, mPipelineState(nullptr)
	{
	}


	/// <summary>
	/// 深度ステンシルの初期設定の取得
	/// </summary>
	/// <returns></returns>
	D3D12_DEPTH_STENCIL_DESC SpritePipeline::GetDepthStencilDesc()
	{
		D3D12_DEPTH_STENCIL_DESC desc = {};
		desc.DepthEnable = FALSE;
		desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		desc.StencilEnable = FALSE;
		desc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
		desc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

		desc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		desc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		desc.BackFace = desc.FrontFace;

		return desc;
	}

	/// <summary>
	/// ブレンドステートの初期設定の取得
	/// </summary>
	/// <returns></returns>
	D3D12_BLEND_DESC SpritePipeline::GetBlendStateDesc()
	{
		D3D12_BLEND_DESC desc = {};
		desc.AlphaToCoverageEnable = FALSE;
		desc.IndependentBlendEnable = FALSE;
		D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
		renderTargetBlendDesc.BlendEnable = TRUE;
		renderTargetBlendDesc.LogicOpEnable = FALSE;
		renderTargetBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		renderTargetBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		renderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
		renderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
		renderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
		renderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		renderTargetBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
		renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		for (auto& rt : desc.RenderTarget)
		{
			rt = renderTargetBlendDesc;
		}
		return desc;
	}

	/// <summary>
	/// ラスタライザーの初期設定の取得
	/// </summary>
	/// <returns></returns>
	D3D12_RASTERIZER_DESC SpritePipeline::GetRasterizerDesc()
	{
		D3D12_RASTERIZER_DESC desc = {};
		desc.FillMode = D3D12_FILL_MODE_SOLID;
		desc.CullMode = D3D12_CULL_MODE_NONE;
		desc.FrontCounterClockwise = FALSE;
		desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		desc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		desc.DepthClipEnable = TRUE;
		desc.MultisampleEnable = FALSE;
		desc.AntialiasedLineEnable = FALSE;
		desc.ForcedSampleCount = 0;
		desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		desc.DepthBias = 0;
		desc.SlopeScaledDepthBias = 0.0f;
		desc.DepthBiasClamp = 0.0f;
		return desc;
	}

	/// <summary>
	/// ルートシグネチャの作成
	/// </summary>
	/// <returns>true:成功</returns>
	bool SpritePipeline::CreateRootSignature()
	{
		// t0:テクスチャ
		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		// ルートパラメーター
		CD3DX12_ROOT_PARAMETER1 rootParams[2];
		// 処理速度が気になりだしたら動的CBに変え
		// 0:CBV (b0)
		rootParams[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
		// 1: DescriptorTable (t0)
		rootParams[1].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

		//	 静的サンプラー
		CD3DX12_STATIC_SAMPLER_DESC sampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

		// シグネチャ作成
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
		rootSigDesc.Init_1_1(_countof(rootParams), rootParams, 1, &sampler,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		Blob signatureBlob, errorBlob;
		HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSigDesc, &signatureBlob, &errorBlob);

		if (FAILED(hr))
		{
			ECSE_LOG(System::ELogLevel::Error, "Failed SerializeRootSignature");
			return false;
		}

		auto device = System::ServiceLocator::Get<DX12>()->GetDevice();
		hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
			signatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature));
		if (FAILED(hr))
		{
			ECSE_LOG(System::ELogLevel::Error, "Failed CreateRootSignature");
			return false;
		}

		return true;
	}

	/// <summary>
	/// 描画パイプラインの作成
	/// </summary>
	/// <returns></returns>
	bool SpritePipeline::CreatePipeline()
	{
		auto shaderManager = System::ServiceLocator::Get<ShaderManager>();
		auto VS = shaderManager->GetShader("Assets/Shader/VS_Texture.hlsl", "main", "vs_5_0");
		auto PS = shaderManager->GetShader("Assets/Shader/PS_Texture.hlsl", "main", "ps_5_0");

		if (VS == nullptr || PS == nullptr)
		{
			ECSE_LOG(System::ELogLevel::Error, "Failed GetShader.");
			return false;
		}

		// インプットレイアウト（明示的に12byte目からを指定します）
		// // RootSignature layout:
		// b0 : SpriteConstantBuffer (WVP, Color, Ind)
		// t0 : SpriteTexture
		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		// パイプラインステート構築
		CD3DX12_BLEND_DESC blendDesc(D3D12_DEFAULT);
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

		CD3DX12_RASTERIZER_DESC rasterDesc(D3D12_DEFAULT);
		rasterDesc.CullMode = D3D12_CULL_MODE_NONE; // 両面描画（スプライトの反転等に対応）

		CD3DX12_DEPTH_STENCIL_DESC depthDesc(D3D12_DEFAULT);
		depthDesc.DepthEnable = FALSE; // 2Dなので深度テストは基本オフ

		// PSOの設定
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.pRootSignature = mRootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(VS.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(PS.Get());
		psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
		psoDesc.BlendState = blendDesc;
		psoDesc.RasterizerState = rasterDesc;
		psoDesc.DepthStencilState = depthDesc;

		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;

		// PSOの作成
		auto device = System::ServiceLocator::Get<DX12>()->GetDevice();
		HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPipelineState));

		if (FAILED(hr))
		{
			ECSE_LOG(System::ELogLevel::Error, "Failed Create PSO.");
			return false;
		}

		// デバッカー用に名前つけ
		mPipelineState->SetName(L"Ecse::SpritePipelinePSO");

		return true;
	}

	/// <summary>
	/// 初期化
	/// </summary>
	/// <returns></returns>
	bool SpritePipeline::Create()
	{
		bool ret = false;
		ret = this->CreateRootSignature();
		if (ret == false)
		{
			ECSE_LOG(System::ELogLevel::Error, "Failed CreateRootSignature.");
			return false;
		}

		ret = this->CreatePipeline();
		if (ret == false)
		{
			ECSE_LOG(System::ELogLevel::Error, "Failed CreatePipeline.");
			return false;
		}

		return true;
	}

	/// <summary>
	/// ルートシグネチャの取得
	/// </summary>
	/// <returns></returns>
	ID3D12RootSignature* SpritePipeline::GetRootSignature() const
	{
		return mRootSignature.Get();
	}

	/// <summary>
	/// パイプラインステートの取得
	/// </summary>
	/// <returns></returns>
	ID3D12PipelineState* SpritePipeline::GetPipelineState() const
	{
		return mPipelineState.Get();
	}
}

