#include "pch.h"
#include<ECS/Component/FBX/FbxComponent.hpp>
#include<Graphics/FBX/Resource/FbxResource.hpp>

void Ecse::ECS::FbxComponent::SetResource(const Graphics::FbxResource* res)
{
    Resource = res;

    if (!Resource) {
        BoneTransforms.clear();
        return;
    }

    // リソースからボーン情報を取得
    const auto bones = Resource->GetBones();
    const size_t boneCount = bones.size();

    // 行列配列をリサイズ（XMMATRIXは16バイト整列が必要な場合があるため、
    // 必要に応じてアライメント済みのベクター検討もアリですが、
    // std::vectorはC++17以降、型のアライメントを考慮します）
    BoneTransforms.resize(boneCount);

    // 単位行列で初期化
    const DirectX::XMMATRIX identity = DirectX::XMMatrixIdentity();
    for (auto& m : BoneTransforms) {
        DirectX::XMStoreFloat4x4(&m, identity);
    }
}

void Ecse::ECS::AnimatorComponent::Play(const std::string& name, bool loop, float speed)
{
    if (CurrentAnimationName == name) return;
    NextAnimationName = name;
    TransitionTime = 0.0f; // ブレンド開始

}
