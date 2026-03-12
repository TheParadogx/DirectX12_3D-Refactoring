#include "pch.h"
#include "FbxPipeline.hpp"
#include <Graphics/DX12/DX12.hpp>
#include <Graphics/Shader/ShaderManager.hpp>

namespace Ecse::Graphics
{
    FbxPipeline::FbxPipeline() : mRootSignature(nullptr), mPipelineState(nullptr) {}

    bool FbxPipeline::Create()
    {
        if (!CreateRootSignature()) return false;
        if (!CreatePipeline()) return false;
        return true;
    }

    bool FbxPipeline::CreateRootSignature()
    {
        // パラメータ構成を整理 (以前の設計 + ボーン用)
        // 0: Scene(b3), 1: Mesh(b4), 2: Bones(b5), 3: Texture(t0)
        std::array<CD3DX12_ROOT_PARAMETER1, 4> rootParams;

        rootParams[0].InitAsConstantBufferView(3); // register(b3)
        rootParams[1].InitAsConstantBufferView(4); // register(b4)
        rootParams[2].InitAsConstantBufferView(5); // register(b5)

        CD3DX12_DESCRIPTOR_RANGE1 range;
        range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // register(t0)
        rootParams[3].InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_PIXEL);

        CD3DX12_STATIC_SAMPLER_DESC sampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
        rootSigDesc.Init_1_1(static_cast<UINT>(rootParams.size()), rootParams.data(), 1, &sampler,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        Blob signatureBlob, errorBlob;
        HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSigDesc, &signatureBlob, &errorBlob);
        if (FAILED(hr)) return false;

        auto device = System::ServiceLocator::Get<DX12>()->GetDevice();
        device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature));

        return true;
    }

    bool FbxPipeline::CreatePipeline()
    {
        auto shaderManager = System::ServiceLocator::Get<ShaderManager>();
        auto VS = shaderManager->GetShader("EngineAssets/Shader/VS_Fbx.hlsl", "VS_main", "vs_5_0");
        auto PS = shaderManager->GetShader("EngineAssets/Shader/PS_Fbx.hlsl", "PS_main", "ps_5_0");
        if (!VS || !PS) return false;

        // 前のプロジェクトの形式に合わせたインプットレイアウト
        D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
            { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "BONE_INDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT,  0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "WEIGHT",     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
        psoDesc.pRootSignature = mRootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(VS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(PS.Get());
        psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };

        // 前のプロジェクトと同じデフォルト設定を使用
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // 形を確認するため一旦カリングOFF
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.SampleDesc.Count = 1;

        auto device = System::ServiceLocator::Get<DX12>()->GetDevice();
        device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPipelineState));

        return true;
    }

	ID3D12RootSignature* FbxPipeline::GetRootSignature() const { return mRootSignature.Get(); }
	ID3D12PipelineState* FbxPipeline::GetPipelineState() const { return mPipelineState.Get(); }
}