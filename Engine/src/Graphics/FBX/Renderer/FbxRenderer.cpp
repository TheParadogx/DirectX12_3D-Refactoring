#include "pch.h"
#include "FbxRenderer.hpp"
#include <Graphics/FBX/Resource/FbxResource.hpp>
#include <Graphics/DX12/DX12.hpp>
#include <ECS/Component/Transform/TransformComponent.hpp>
#include <ECS/Component/FBX/FbxComponent.hpp>
#include <Graphics/Texture/Texture.hpp>
#include <Graphics/GraphicsDescriptorHeap/GDescriptorHeapManager.hpp>

using namespace DirectX;

namespace Ecse::Graphics
{
    bool FbxRenderer::Initialize()
    {
        auto device = System::ServiceLocator::Get<DX12>()->GetDevice();
        auto frameCount = System::ServiceLocator::Get<DX12>()->FRAME_COUNT;

        mConstantBuffer = std::make_unique<DynamicBuffer>(device, 8 * 1024 * 1024, frameCount);

        mPipeline = std::make_unique<FbxPipeline>();
        if (!mPipeline->Create())
        {
            return false;
        }

        return true;
    }

    void FbxRenderer::Render(entt::registry& registry, ID3D12GraphicsCommandList* cmdList, uint32_t frameIndex, const XMMATRIX& viewProj)
    {
        mConstantBuffer->Reset(frameIndex);

        auto heapManager = System::ServiceLocator::Get<GDescriptorHeapManager>();
        ID3D12DescriptorHeap* heaps[] = { heapManager->GetNativeHeap() };
        cmdList->SetDescriptorHeaps(_countof(heaps), heaps);

        cmdList->SetGraphicsRootSignature(mPipeline->GetRootSignature());
        cmdList->SetPipelineState(mPipeline->GetPipelineState());
        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        SceneConstant scene;
        XMStoreFloat4x4(&scene.viewProjection, XMMatrixTranspose(viewProj));
        cmdList->SetGraphicsRootConstantBufferView(0, mConstantBuffer->Upload(frameIndex, scene));

        auto view = registry.view<ECS::Transform3D, ECS::FbxComponent>();

        view.each([&](auto entity, ECS::Transform3D& transform, ECS::FbxComponent& fbx)
            {
                if (!fbx.Resource) return;

                MeshConstant mesh;
                XMStoreFloat4x4(&mesh.world, XMMatrixTranspose(transform.GetWorldMatrix()));
                cmdList->SetGraphicsRootConstantBufferView(1, mConstantBuffer->Upload(frameIndex, mesh));

                BoneConstant boneData{};
                const auto& sourceBones = fbx.BoneTransforms.empty()
                    ? fbx.Resource->GetDefaultBoneTransforms()
                    : fbx.BoneTransforms;

                uint32_t copyCount = (uint32_t)std::min<size_t>(sourceBones.size(), MAX_BONES);
                for (uint32_t i = 0; i < copyCount; ++i)
                {
                    XMStoreFloat4x4(&boneData.bones[i], XMMatrixTranspose(XMLoadFloat4x4(&sourceBones[i])));
                }
                cmdList->SetGraphicsRootConstantBufferView(2, mConstantBuffer->Upload(frameIndex, boneData));

                fbx.Resource->SetBuffers(cmdList);

                for (const auto& mat : fbx.Resource->GetMaterials())
                {
                    if (mat.Texture)
                    {
                        cmdList->SetGraphicsRootDescriptorTable(3, mat.Texture->GetGpuHandle());
                    }

                    cmdList->DrawIndexedInstanced(mat.IndexCount, 1, mat.StartIndex, 0, 0);
                }
            });
    }
}