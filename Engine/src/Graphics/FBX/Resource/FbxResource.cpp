#include "pch.h"
#include <Graphics/FBX/Resource/FbxResource.hpp>
#include <Graphics/Texture/Texture.hpp>
#include <Graphics/Texture/Manager/TextureManager.hpp>
#include <Graphics/IndexBuffer/IndexBuffer.hpp>
#include <Graphics/VertexBuffer/VertexBuffer.hpp>
#include <Graphics/Data/GraphicsData.hpp>
#include <fstream>

using namespace DirectX;

namespace Ecse::Graphics
{
    FbxResource::FbxResource() : mVB(nullptr), mIB(nullptr) {}
    FbxResource::~FbxResource() { Release(); }

    void FbxResource::Release()
    {
        mMaterials.clear();
        mBones.clear();
        mAnimations.clear();
        mBoneIndexMap.clear();
    }

    void FbxResource::SetBuffers(ID3D12GraphicsCommandList* CmdList) const
    {
        if (mVB) mVB->Set(CmdList);
        if (mIB) mIB->Set(CmdList);
    }

    std::vector<DirectX::XMFLOAT4X4> FbxResource::GetDefaultBoneTransforms() const
    {
        std::vector<DirectX::XMFLOAT4X4> identities(mBones.size());
        for (auto& mat : identities) {
            XMStoreFloat4x4(&mat, XMMatrixIdentity());
        }
        return identities;
    }

    std::span<const std::vector<DirectX::XMFLOAT4X4>> FbxResource::GetAnimationKeyFrames(const std::string& name) const
    {
        auto it = mAnimations.find(name);
        if (it != mAnimations.end()) return it->second.KeyFrame;
        return {};
    }

    D3D12_VERTEX_BUFFER_VIEW FbxResource::GetVertexBufferView() const { return mVB ? mVB->GetView() : D3D12_VERTEX_BUFFER_VIEW{}; }
    D3D12_INDEX_BUFFER_VIEW FbxResource::GetIndexBufferView() const { return mIB ? mIB->GetView() : D3D12_INDEX_BUFFER_VIEW{}; }

    bool FbxResource::Create(const std::filesystem::path& FilePath)
    {
        Release();
        return LoadModelData(FilePath);
    }

    // --- アニメーション読み込み ---
    bool FbxResource::LoadAnimation(const std::string& AnimationName, const std::filesystem::path& AnimationPath)
    {
        std::ifstream ifs(AnimationPath, std::ios::binary);
        if (!ifs) return false;

        char magic[4];
        ifs.read(magic, 4);
        if (strncmp(magic, "ANIM", 4) != 0) return false;

        uint32_t bCount, fCount;
        float duration;
        ifs.read((char*)&bCount, 4);
        ifs.read((char*)&fCount, 4);
        ifs.read((char*)&duration, 4);

        Animation anim;
        anim.NumFrame = fCount;
        anim.Duration = duration;
        anim.KeyFrame.resize(fCount);

        for (auto& frame : anim.KeyFrame) {
            float time;
            ifs.read((char*)&time, 4);
            frame.resize(bCount);
            for (uint32_t i = 0; i < bCount; ++i) {
                // ★修正：行優先(Row-Major)なので転置せずそのまま読み込む
                ifs.read(reinterpret_cast<char*>(&frame[i]), 64);
            }
        }
        mAnimations[AnimationName] = std::move(anim);
        return true;
    }

    // --- モデル本体の読み込み ---
    bool FbxResource::LoadModelData(const std::filesystem::path& FilePath)
    {
        std::ifstream ifs(FilePath, std::ios::binary);
        if (!ifs) return false;

        uint32_t head[5];
        ifs.read(reinterpret_cast<char*>(head), 20);
        if (head[0] != 0x4D4F444C) return false; // "MODL"

        uint32_t vCount = head[2];
        uint32_t iCount = head[3];
        uint32_t bCount = head[4];

        // 頂点バッファ作成
        std::vector<SkeletalMeshVertex> vertices(vCount);
        for (auto& v : vertices) {
            ifs.read(reinterpret_cast<char*>(&v), sizeof(SkeletalMeshVertex));
        }
        mVB = std::make_unique<VertexBuffer>();
        if (!mVB->Create(sizeof(SkeletalMeshVertex) * vCount, sizeof(SkeletalMeshVertex))) return false;
        mVB->Update(vertices.data(), sizeof(SkeletalMeshVertex) * vCount);

        // インデックスバッファ作成
        std::vector<uint32_t> indices(iCount);
        ifs.read(reinterpret_cast<char*>(indices.data()), sizeof(uint32_t) * iCount);
        mIB = std::make_unique<IndexBuffer>();
        if (!mIB->Create(sizeof(uint32_t) * iCount, eIndexBufferFormat::Uint32)) return false;
        mIB->Update(indices.data(), sizeof(uint32_t) * iCount);

        // マテリアル読み込み
        uint32_t sCount = 0;
        ifs.read(reinterpret_cast<char*>(&sCount), 4);
        mMaterials.resize(sCount);
        for (auto& mat : mMaterials) {
            uint32_t matIdx;
            ifs.read(reinterpret_cast<char*>(&matIdx), 4);
            ifs.read(reinterpret_cast<char*>(&mat.StartIndex), 4);
            ifs.read(reinterpret_cast<char*>(&mat.IndexCount), 4);
        }

        // ボーン読み込み
        mBones.resize(bCount);
        for (auto& b : mBones) {
            uint32_t nLen;
            ifs.read(reinterpret_cast<char*>(&nLen), 4);
            b.Name.resize(nLen);
            ifs.read(&b.Name[0], nLen);
            ifs.read(reinterpret_cast<char*>(&b.ParentIndex), 4);

            // ★修正：OffsetMatrixも転置せずそのまま読み込む
            // ファイル内の行列形式を統一して扱うため
            ifs.read(reinterpret_cast<char*>(&b.OffsetMatrix), 64);

            b.BindMatrix = b.OffsetMatrix;
        }

        // テクスチャ読み込み
        uint32_t mCount = 0;
        ifs.read(reinterpret_cast<char*>(&mCount), 4);
        std::filesystem::path baseDir = FilePath.parent_path();
        TextureManager* Manager = System::ServiceLocator::Get<TextureManager>();

        for (uint32_t i = 0; i < mCount; ++i) {
            uint32_t nL, tL;
            std::string mName, tPath;
            ifs.read(reinterpret_cast<char*>(&nL), 4); mName.resize(nL); ifs.read(&mName[0], nL);
            ifs.read(reinterpret_cast<char*>(&tL), 4); tPath.resize(tL); ifs.read(&tPath[0], tL);
            if (i < mMaterials.size()) {
                mMaterials[i].Name = mName;
                auto path = baseDir / tPath;
                mMaterials[i].Texture = Manager->GetOrLoad(path);
            }
        }
        return true;
    }
}