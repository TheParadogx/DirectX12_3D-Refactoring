#include "pch.h"
#include "FbxAnimationSystem.hpp"
#include<ECS/Component/FBX/FbxComponent.hpp>
#include<Graphics/FBX/Resource/FbxResource.hpp>

using namespace DirectX;

void Ecse::System::FbxAnimationSystem::Update(entt::registry& registry, float deltaTime)
{
    auto view = registry.view<ECS::FbxComponent>();

    // 並列更新を考慮したスレッドローカルバッファ
    thread_local std::vector<XMMATRIX> sWorldMatrices;

    for (auto entity : view) {
        auto& fbx = view.get<ECS::FbxComponent>(entity);

        if (!fbx.Resource || fbx.CurrentAnimationName.empty()) continue;

        // アニメーションスパン取得
        auto animFrames = fbx.Resource->GetAnimationKeyFrames(fbx.CurrentAnimationName);
        if (animFrames.empty()) continue;

        const auto& bones = fbx.Resource->GetBones();
        size_t boneCount = bones.size();

        // バッファの整合性チェック
        if (fbx.BoneTransforms.size() != boneCount) fbx.BoneTransforms.resize(boneCount);
        if (sWorldMatrices.size() < boneCount) sWorldMatrices.resize(boneCount);

        // 時間更新とフレーム決定
        fbx.CurrentTime += deltaTime * fbx.PlaySpeed;
        int frameCount = static_cast<int>(animFrames.size());
        int currentFrame = static_cast<int>(fbx.CurrentTime * 30.0f);

        if (fbx.IsLoop) currentFrame %= frameCount;
        else currentFrame = (std::min)(currentFrame, frameCount - 1);

        const std::vector<XMFLOAT4X4>& currentKeyFrames = animFrames[currentFrame];

        // スケルトン階層計算
        for (size_t i = 0; i < boneCount; ++i) {
            // 現在のボーンのローカル変換をロード
            XMMATRIX local = XMLoadFloat4x4(&currentKeyFrames[i]);

            // childWorld = parentWorld * childLocal
            if (bones[i].ParentIndex >= 0) {
                local = XMMatrixMultiply(
                    sWorldMatrices[bones[i].ParentIndex],
                    local
                );
            }

            // 子ボーンの参照用に保存
            sWorldMatrices[i] = local;

            // Final = InverseBindPose * World
            XMMATRIX bindInv = XMLoadFloat4x4(&bones[i].BindMatrix);
            XMMATRIX finalMat = XMMatrixMultiply(bindInv, local);

            // HLSL用に転置
            XMStoreFloat4x4(&fbx.BoneTransforms[i], XMMatrixTranspose(finalMat));
        }
    }
}
