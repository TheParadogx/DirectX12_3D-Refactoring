#include "pch.h"
#include "FbxAnimationSystem.hpp"
#include <ECS/Component/FBX/FbxComponent.hpp>
#include <Graphics/FBX/Resource/FbxResource.hpp>

using namespace DirectX;

namespace Ecse::System {

    void FbxAnimationSystem::Update(entt::registry& registry, float deltaTime)
    {
        auto view = registry.view<ECS::FbxComponent, ECS::AnimatorComponent>();

        view.each([&](auto entity, ECS::FbxComponent& fbx, ECS::AnimatorComponent& animator)
            {
                if (!fbx.Resource || !animator.IsPlaying || animator.CurrentAnimationName.empty()) return;

                const auto& animations = fbx.Resource->GetAnimations();
                auto it = animations.find(animator.CurrentAnimationName);
                if (it == animations.end()) return;

                const auto& animData = it->second;

                animator.CurrentTime = std::fmod(animator.CurrentTime + deltaTime * animator.PlaySpeed, animData.Duration);
                if (animator.CurrentTime < 0.0f) animator.CurrentTime += animData.Duration;

                const float framePos = animator.CurrentTime * animData.FPS;
                const uint32_t f0 = static_cast<uint32_t>(std::floor(framePos)) % animData.NumFrame;
                const uint32_t f1 = (f0 + 1u) % animData.NumFrame;
                const float weight = framePos - std::floor(framePos);

                const auto& bones = fbx.Resource->GetBones();
                const size_t numBones = bones.size();
                fbx.BoneTransforms.resize(numBones);

                std::vector<XMMATRIX> currentGlobals(numBones);

                for (size_t i = 0; i < numBones; ++i)
                {
                    XMVECTOR s0, r0, t0, s1, r1, t1;
                    XMMatrixDecompose(&s0, &r0, &t0, XMLoadFloat4x4(&animData.KeyFrame[f0][i]));
                    XMMatrixDecompose(&s1, &r1, &t1, XMLoadFloat4x4(&animData.KeyFrame[f1][i]));

                    XMVECTOR s = XMVectorLerp(s0, s1, weight);
                    XMVECTOR r = XMQuaternionSlerp(r0, r1, weight);
                    XMVECTOR t = XMVectorLerp(t0, t1, weight);

                    XMMATRIX local = XMMatrixScalingFromVector(s) * XMMatrixRotationQuaternion(r) * XMMatrixTranslationFromVector(t);

                    const int pIdx = bones[i].ParentIndex;
                    if (pIdx < 0) {
                        currentGlobals[i] = local;
                    }
                    else {
                        currentGlobals[i] = XMMatrixMultiply(local, currentGlobals[pIdx]);
                    }

                    XMMATRIX offset = XMLoadFloat4x4(&bones[i].OffsetMatrix);
                    
                    XMMATRIX finalTransform = XMMatrixMultiply(offset, currentGlobals[i]);
                    
                    XMStoreFloat4x4(&fbx.BoneTransforms[i], finalTransform);

                    if (i == 0) {
                        XMFLOAT4X4 checkF;
                        XMStoreFloat4x4(&checkF, finalTransform);
                        char buf[256];
                        sprintf_s(buf, "Bone:0 X:%.2f, Y:%.2f, Z:%.2f\n", checkF._41, checkF._42, checkF._43);
                        OutputDebugStringA(buf);
                    }
                }
            });
    }
}