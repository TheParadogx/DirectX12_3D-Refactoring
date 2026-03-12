#pragma once
#include <Utility/Export/Export.hpp>
#include <Utility/Types/EcseTypes.hpp>
#include <DirectXMath.h>

#include <string>
#include <vector>
#include <filesystem>
#include <span>
#include <memory>
#include <unordered_map>

namespace Ecse::Graphics
{
    class Texture;
    class IndexBuffer;
    class VertexBuffer;

    /// <summary>
    /// 旧プロジェクトのFbxAnalyzerバイナリに対応したリソース管理クラス
    /// </summary>
    class ENGINE_API FbxResource
    {
    public:
        struct Material
        {
            std::string Name;
            Texture* Texture = nullptr;
            uint32_t IndexCount = 0;
            uint32_t StartIndex = 0;
        };

        struct BoneInfo
        {
            std::string Name;
            int32_t ParentIndex;
            DirectX::XMFLOAT4X4 BindMatrix; // ファイル内の行列(唯一)
        };

        struct Animation
        {
            int32_t NumFrame = 0;
            // KeyFrames[ボーンIndex][フレーム番号] 
            std::vector<std::vector<DirectX::XMFLOAT4X4>> KeyFrames;
        };

    public:
        FbxResource();
        virtual ~FbxResource();

        /// <summary>
        /// FBXモデル(.bin)の読込
        /// </summary>
        bool Create(const std::filesystem::path& FilePath);

        /// <summary>
        /// アニメーション(.anm)の読込
        /// </summary>
        bool LoadAnimation(const std::string& AnimationName, const std::filesystem::path& AnimationPath);

        /// <summary>
        /// リソースの解放
        /// </summary>
        void Release();

        /// <summary>
        /// バッファをコマンドリストにセット
        /// </summary>
        void SetBuffers(ID3D12GraphicsCommandList* CmdList) const;

        /// <summary>
        /// 未解決シンボル解消：デフォルトの単位行列配列を生成
        /// </summary>
        std::vector<DirectX::XMFLOAT4X4> GetDefaultBoneTransforms() const;

        // ゲッター
        std::span<const Material> GetMaterials() const { return mMaterials; }
        std::span<const BoneInfo> GetBones() const { return mBones; }
        const std::unordered_map<std::string, Animation>& GetAnimations() const { return mAnimations; }

        // ボーンを持っているか（スキニングが必要か）
        bool HasBones() const { return !mBones.empty(); }

    private:
        bool LoadModelData(const std::filesystem::path& FilePath);

    private:
        std::vector<Material> mMaterials;
        std::vector<BoneInfo> mBones;
        std::unordered_map<std::string, Animation> mAnimations;

        std::unique_ptr<VertexBuffer> mVB;
        std::unique_ptr<IndexBuffer> mIB;
    };
}