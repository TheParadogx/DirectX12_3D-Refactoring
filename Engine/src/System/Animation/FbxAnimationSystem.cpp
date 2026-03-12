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

            // 時間の更新
            animator.CurrentTime += deltaTime * animator.PlaySpeed;

            // 行列の計算
            fbx.Resource->ComputeAnimationTransforms(
                animator.CurrentAnimationName,
                animator.CurrentTime,
                fbx.BoneTransforms
            );
           });
    }
}