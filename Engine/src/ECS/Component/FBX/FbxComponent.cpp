#include "pch.h"
#include<ECS/Component/FBX/FbxComponent.hpp>
#include<Graphics/FBX/Resource/FbxResource.hpp>

void Ecse::ECS::FbxComponent::SetResource(const Graphics::FbxResource* res)
{
    this->Resource = res;
    if (!res) return;

    // リソースからボーン情報を取得
    auto bones = res->GetBones();
    BoneTransforms.resize(bones.size());

    // 初期ポーズとして BindMatrix をコピー
    for (size_t i = 0; i < bones.size(); ++i) {
        BoneTransforms[i] = bones[i].BindMatrix;
    }
}

void Ecse::ECS::AnimatorComponent::Play(const std::string& name, bool loop, float speed)
{
    if (CurrentAnimationName == name) return;
    NextAnimationName = name;
    TransitionTime = 0.0f; // ブレンド開始

}
