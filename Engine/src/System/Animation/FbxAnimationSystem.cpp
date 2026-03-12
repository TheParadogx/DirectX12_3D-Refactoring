#include "pch.h"
#include "FbxAnimationSystem.hpp"
#include <ECS/Component/FBX/FbxComponent.hpp>
#include <Graphics/FBX/Resource/FbxResource.hpp>

using namespace DirectX;

namespace Ecse::System {

    void FbxAnimationSystem::Update(entt::registry& registry, float deltaTime)
    {
        auto view = registry.view<ECS::FbxComponent, ECS::AnimatorComponent>();

        view.each([&](auto entity, ECS::FbxComponent& fbx, ECS::AnimatorComponent& animator) {
            if (!fbx.Resource || !animator.IsPlaying) return;

            // 現在のアニメーション時間を進める
            animator.CurrentTime += deltaTime * animator.PlaySpeed;

            // クロスフェード中かどうかの判定
            if (!animator.NextAnimationName.empty())
            {
                // 次のアニメーション時間（遷移開始からの時間）
                animator.TransitionTime += deltaTime;
                float t = std::clamp(animator.TransitionTime / animator.TransitionDuration, 0.0f, 1.0f);

                // 1. 現在のアニメーションのポーズを計算
                std::vector<XMFLOAT4X4> currentPose;
                fbx.Resource->ComputeAnimationTransforms(animator.CurrentAnimationName, animator.CurrentTime, currentPose);

                // 2. 次のアニメーションのポーズを計算 (TransitionTime をそのまま時間として使う)
                std::vector<XMFLOAT4X4> nextPose;
                fbx.Resource->ComputeAnimationTransforms(animator.NextAnimationName, animator.TransitionTime, nextPose);

                // 3. 2つのポーズを混ぜる
                fbx.Resource->BlendPoses(currentPose, nextPose, t, fbx.BoneTransforms);

                // 遷移完了処理
                if (t >= 1.0f) {
                    animator.CurrentAnimationName = animator.NextAnimationName;
                    animator.CurrentTime = animator.TransitionTime;
                    animator.NextAnimationName = "";
                    animator.TransitionTime = 0.0f;
                }
            }
            else
            {
                // 通常再生
                fbx.Resource->ComputeAnimationTransforms(animator.CurrentAnimationName, animator.CurrentTime, fbx.BoneTransforms);
            }
            });
    }
}