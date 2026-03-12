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

        // 定数バッファのメモリ確保
        mConstantBuffer = std::make_unique<DynamicBuffer>(device, 8 * 1024 * 1024, frameCount);

        mPipeline = std::make_unique<FbxPipeline>();
        if (!mPipeline->Create()) return false;

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

        // --- Slot 0: Scene Constant (b3) ---
        SceneConstant scene;
        XMStoreFloat4x4(&scene.viewProjection, XMMatrixTranspose(viewProj));
        cmdList->SetGraphicsRootConstantBufferView(0, mConstantBuffer->Upload(frameIndex, scene));

        auto view = registry.view<ECS::Transform3D, ECS::FbxComponent>();
        view.each([&](auto entity, ECS::Transform3D& transform, ECS::FbxComponent& fbx)
            {
                if (!fbx.Resource) return;

                // --- Slot 1: Mesh Constant (b4) ---
                MeshConstant mesh;
                XMStoreFloat4x4(&mesh.world, XMMatrixTranspose(transform.GetWorldMatrix()));
                mesh.color = { 1.0f, 1.0f, 1.0f, 1.0f }; // Component等から取得可能なら変更
                mesh.isSkinned = fbx.Resource->HasBones() ? 1 : 0;
                cmdList->SetGraphicsRootConstantBufferView(1, mConstantBuffer->Upload(frameIndex, mesh));

                // --- Slot 2: Bone Constant (b5) ---
                if (mesh.isSkinned) {
                    BoneConstant boneData{};

                    // データがなければデフォルト姿勢
                    if (fbx.BoneTransforms.empty()) {
                        auto defaults = fbx.Resource->GetDefaultBoneTransforms();
                        for (size_t i = 0; i < std::min(defaults.size(), (size_t)MAX_BONES); ++i) {
                            XMStoreFloat4x4(&boneData.bones[i], XMMatrixTranspose(XMLoadFloat4x4(&defaults[i])));
                        }
                    }
                    else {
                        for (size_t i = 0; i < std::min(fbx.BoneTransforms.size(), (size_t)MAX_BONES); ++i) {
                            XMStoreFloat4x4(&boneData.bones[i], XMMatrixTranspose(XMLoadFloat4x4(&fbx.BoneTransforms[i])));
                        }
                    }
                    cmdList->SetGraphicsRootConstantBufferView(2, mConstantBuffer->Upload(frameIndex, boneData));
                }

                // --- Draw ---
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