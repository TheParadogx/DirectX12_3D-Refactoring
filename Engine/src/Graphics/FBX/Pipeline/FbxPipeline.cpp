#include "pch.h"
#include "FbxPipeline.hpp"
#include<Graphics/DX12/DX12.hpp>
#include<Graphics/Shader/ShaderManager.hpp>

namespace Ecse::Graphics
{
	FbxPipeline::FbxPipeline()
		:mRootSignature(nullptr)
		, mPipelineState(nullptr)
	{
	}

	bool FbxPipeline::Create()
	{
		if (!CreateRootSignature()) return false;
		if (!CreatePipeline()) return false;
		return true;
	}

	/// <summary>
	/// 3D描画用の深度設定（Zテスト・Zライト有効）
	/// </summary>
	D3D12_DEPTH_STENCIL_DESC FbxPipeline::GetDepthStencilDesc()
	{
		CD3DX12_DEPTH_STENCIL_DESC desc(D3D12_DEFAULT);
		desc.DepthEnable = TRUE;
		desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		desc.StencilEnable = FALSE;
		return desc;
	}

	/// <summary>
	/// 3D描画用のブレンド設定（基本は不透明、必要に応じてアルファテスト）
	/// </summary>
	D3D12_BLEND_DESC FbxPipeline::GetBlendStateDesc()
	{
		// 3Dモデルは基本不透明描画として設定
		CD3DX12_BLEND_DESC desc(D3D12_DEFAULT);
		return desc;
	}

	/// <summary>
	/// 3D描画用のラスタライザー設定（背面カリング有効）
	/// </summary>
	D3D12_RASTERIZER_DESC FbxPipeline::GetRasterizerDesc()
	{
		CD3DX12_RASTERIZER_DESC desc(D3D12_DEFAULT);
		desc.CullMode = D3D12_CULL_MODE_BACK; // 背面カリング
		desc.FillMode = D3D12_FILL_MODE_SOLID;
		return desc;
	}

	bool FbxPipeline::CreateRootSignature()
	{
		// 1. Descriptor Range (t0用)
		CD3DX12_DESCRIPTOR_RANGE1 rangeTex;
		rangeTex.Init(
			D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			1, 0, 0,
			D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC
		);

		// 2. Root Parameters
		CD3DX12_ROOT_PARAMETER1 rootParams[4];

		// b3: Scene/Transform
		rootParams[0].InitAsConstantBufferView(3, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
		// b4: Mesh/World
		rootParams[1].InitAsConstantBufferView(4, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
		// b5: Bones
		rootParams[2].InitAsConstantBufferView(5, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
		// t0: Texture (これを追加し忘れていました)
		rootParams[3].InitAsDescriptorTable(1, &rangeTex, D3D12_SHADER_VISIBILITY_PIXEL);

		// 3. Static Sampler (s0用)
		CD3DX12_STATIC_SAMPLER_DESC sampler(
			0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP
		);

		// 4. Flags
		D3D12_ROOT_SIGNATURE_FLAGS flags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		// 5. Serialize
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
		rootSigDesc.Init_1_1(_countof(rootParams), rootParams, 1, &sampler, flags);

		Blob signatureBlob, errorBlob;
		HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSigDesc, &signatureBlob, &errorBlob);
		if (FAILED(hr))
		{
			if (errorBlob) {
				// エラー内容をログに出すとデバッグが捗ります
				std::string err = (char*)errorBlob->GetBufferPointer();
				ECSE_LOG(System::eLogLevel::Error, "RootSig Error: " + err);
			}
			return false;
		}

		auto device = System::ServiceLocator::Get<DX12>()->GetDevice();
		hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
			signatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature));

		if (FAILED(hr))
		{
			ECSE_LOG(System::eLogLevel::Error, "Failed to create root signature.");
			return false;
		}

		return true;
	}

	bool FbxPipeline::CreatePipeline()
	{

		auto shaderManager = System::ServiceLocator::Get<ShaderManager>();
		auto VS = shaderManager->GetShader("EngineAssets/Shader/VS_Fbx.hlsl", "VS_main", "vs_5_0");
		auto PS = shaderManager->GetShader("EngineAssets/Shader/PS_Fbx.hlsl", "PS_main", "ps_5_0");
		if (!VS || !PS) return false;

		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{ "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BONE_INDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT,  0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BONE_WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 60, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		// PSO設定
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{}; // Zero Initialize
		psoDesc.pRootSignature = mRootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(VS.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(PS.Get());
		psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
		psoDesc.BlendState = GetBlendStateDesc();
		psoDesc.RasterizerState = GetRasterizerDesc();
		psoDesc.DepthStencilState = GetDepthStencilDesc();
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // SwapChainと合わせる
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleDesc.Count = 1;

		auto device = System::ServiceLocator::Get<DX12>()->GetDevice();
		HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPipelineState));

		if (SUCCEEDED(hr)) mPipelineState->SetName(L"Ecse::FbxPipelinePSO");
		else
		{
			ECSE_LOG(System::eLogLevel::Error, "Failed to create graphics pipeline state.");
			return false;
		}

		ECSE_LOG(System::eLogLevel::Log, "FbxPipeline created successfully.");
		return true;
	}

	ID3D12RootSignature* FbxPipeline::GetRootSignature() const { return mRootSignature.Get(); }
	ID3D12PipelineState* FbxPipeline::GetPipelineState() const { return mPipelineState.Get(); }

}

