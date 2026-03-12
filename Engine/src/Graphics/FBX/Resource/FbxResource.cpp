#include "pch.h"
#include <Graphics/FBX/Resource/FbxResource.hpp>
#include <Graphics/Texture/Texture.hpp>
#include <Graphics/Texture/Manager/TextureManager.hpp>
#include <Graphics/IndexBuffer/IndexBuffer.hpp>
#include <Graphics/VertexBuffer/VertexBuffer.hpp>
#include <Graphics/Data/GraphicsData.hpp>
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
        mVB.reset();
        mIB.reset();
    }

    bool FbxResource::Create(const std::filesystem::path& FilePath)
    {
        Release();
        return LoadModelData(FilePath);
    }

    bool FbxResource::LoadModelData(const std::filesystem::path& FilePath)
    {
        FILE* fp = nullptr;
        if (fopen_s(&fp, FilePath.string().c_str(), "rb") != 0) return false;

        // 1. ヘッダー読込
        int32_t counts[3]; // Mesh, Poly, Vertex
        fread(counts, sizeof(int32_t), 3, fp);
        uint32_t vertexCount = static_cast<uint32_t>(counts[2]);

        // 2. 頂点データ読込 (コンバータの fwrite 順序を死守)
        std::vector<SkeletalMeshVertex> vertices(vertexCount);
        for (auto& v : vertices) {
            fread(&v.Position, sizeof(DirectX::XMFLOAT3), 1, fp); // Position
            fread(&v.UV, sizeof(DirectX::XMFLOAT2), 1, fp); // UV
            fread(&v.Normal, sizeof(DirectX::XMFLOAT3), 1, fp); // Normal
            fread(v.Bone, sizeof(uint32_t), 4, fp);          // BoneIndices
            fread(v.Weight, sizeof(float), 4, fp);             // Weights
        }

        // 3. インデックスデータ読込
        int32_t indexCount = 0;
        fread(&indexCount, sizeof(int32_t), 1, fp);
        std::vector<uint32_t> indices(indexCount);
        fread(indices.data(), sizeof(uint32_t), indexCount, fp);

        // 4. マテリアル情報の読込
        int32_t materialCount = 0;
        fread(&materialCount, sizeof(int32_t), 1, fp);
        mMaterials.reserve(materialCount);

        std::filesystem::path baseDir = FilePath.parent_path();
        auto* texManager = System::ServiceLocator::Get<TextureManager>();

        uint32_t currentIndexOffset = 0;
        for (int i = 0; i < materialCount; ++i) {
            Material mat = {};
            int32_t nLen, tLen;

            fread(&nLen, sizeof(int32_t), 1, fp);
            mat.Name.resize(nLen);
            fread(mat.Name.data(), 1, nLen, fp);

            fread(&tLen, sizeof(int32_t), 1, fp);
            std::string texPath;
            texPath.resize(tLen);
            fread(texPath.data(), 1, tLen, fp);

            if (!texPath.empty()) {
                // コンバータが分解したファイル名を Texture フォルダから探す
                mat.Texture = texManager->GetOrLoad(baseDir / "Texture" / texPath);
            }

            int32_t matPolyCount = 0;
            fread(&matPolyCount, sizeof(int32_t), 1, fp);
            mat.IndexCount = static_cast<uint32_t>(matPolyCount * 3);
            mat.StartIndex = currentIndexOffset;

            currentIndexOffset += mat.IndexCount;
            mMaterials.push_back(std::move(mat));
        }

        // 5. ボーン情報の読込 (BindMatrix = OffsetMatrix)
        int32_t boneCount = 0;
        fread(&boneCount, sizeof(int32_t), 1, fp);
        mBones.reserve(boneCount);
        for (int i = 0; i < boneCount; ++i) {
            BoneInfo info = {};
            int32_t nLen;
            fread(&nLen, sizeof(int32_t), 1, fp);
            info.Name.resize(nLen);
            fread(info.Name.data(), 1, nLen, fp);

            fread(&info.ParentIndex, sizeof(int32_t), 1, fp);
            fread(&info.BindMatrix, sizeof(DirectX::XMFLOAT4X4), 1, fp);

            mBones.push_back(std::move(info));
        }

        fclose(fp);

        mVB = std::make_unique<VertexBuffer>();
        mVB->Create(sizeof(SkeletalMeshVertex) * vertexCount, sizeof(SkeletalMeshVertex));
        mVB->Update(vertices.data(), sizeof(SkeletalMeshVertex) * vertexCount);

        mIB = std::make_unique<IndexBuffer>();
        mIB->Create(sizeof(uint32_t) * indexCount, eIndexBufferFormat::Uint32);
        mIB->Update(indices.data(), sizeof(uint32_t) * indexCount);

        return true;
    }

    bool FbxResource::LoadAnimation(const std::string& AnimationName, const std::filesystem::path& AnimationPath)
    {
        FILE* fp = nullptr;
        if (fopen_s(&fp, AnimationPath.string().c_str(), "rb") != 0) return false;

        Animation anim = {};
        fread(&anim.NumFrame, sizeof(int32_t), 1, fp);

        int32_t boneCount = 0;
        fread(&boneCount, sizeof(int32_t), 1, fp);

        anim.KeyFrames.resize(boneCount);
        for (int i = 0; i < boneCount; ++i) {
            int32_t frameCount = 0;
            fread(&frameCount, sizeof(int32_t), 1, fp);
            anim.KeyFrames[i].resize(frameCount);
            fread(anim.KeyFrames[i].data(), sizeof(DirectX::XMFLOAT4X4), frameCount, fp);
        }

        fclose(fp);
        mAnimations[AnimationName] = std::move(anim);
        return true;
    }

    std::vector<DirectX::XMFLOAT4X4> FbxResource::GetDefaultBoneTransforms() const
    {
        // Tポーズ用：BindMatrixはリソース側にあるため、アニメーション行列としては単位行列を返す
        std::vector<DirectX::XMFLOAT4X4> identities(mBones.size());
        for (auto& mat : identities) {
            DirectX::XMStoreFloat4x4(&mat, DirectX::XMMatrixIdentity());
        }
        return identities;
    }

    void FbxResource::SetBuffers(ID3D12GraphicsCommandList* CmdList) const
    {
        if (mVB) mVB->Set(CmdList);
        if (mIB) mIB->Set(CmdList);
    }

    void FbxResource::ComputeAnimationTransforms(
        const std::string& animName,
        float time,
        std::vector<DirectX::XMFLOAT4X4>& outTransforms) const
    {
        auto it = mAnimations.find(animName);
        if (it == mAnimations.end()) return;
        const auto& anim = it->second;

        // --- 補間パラメータの計算 ---
        float frameFull = time * 60.0f; // 60fps想定
        uint32_t frame0 = static_cast<uint32_t>(frameFull) % anim.NumFrame;
        uint32_t frame1 = (frame0 + 1) % anim.NumFrame; // 次のフレーム
        float t = frameFull - static_cast<uint32_t>(frameFull); // 小数点部分（補間係数）

        outTransforms.resize(mBones.size());

        for (size_t i = 0; i < mBones.size(); ++i) {
            // 前後のフレーム行列
            XMMATRIX m0 = XMLoadFloat4x4(&anim.KeyFrames[i][frame0]);
            XMMATRIX m1 = XMLoadFloat4x4(&anim.KeyFrames[i][frame1]);

            // 行列をそのまま Lerp すると形が崩れるため、分解して補間する
            XMVECTOR s0, r0, p0, s1, r1, p1;
            XMMatrixDecompose(&s0, &r0, &p0, m0);
            XMMatrixDecompose(&s1, &r1, &p1, m1);

            // スケールと座標は線形補間 (Lerp)
            XMVECTOR s = XMVectorLerp(s0, s1, t);
            XMVECTOR p = XMVectorLerp(p0, p1, t);
            // 回転は球面線形補間 (Slerp)
            XMVECTOR r = XMQuaternionSlerp(r0, r1, t);

            // 再び行列に合成
            XMMATRIX local = XMMatrixScalingFromVector(s) * XMMatrixRotationQuaternion(r) * XMMatrixTranslationFromVector(p);

            // 親子階層の計算
            if (mBones[i].ParentIndex != -1) {
                XMMATRIX parent = XMLoadFloat4x4(&outTransforms[mBones[i].ParentIndex]);
                XMStoreFloat4x4(&outTransforms[i], local * parent);
            }
            else {
                XMStoreFloat4x4(&outTransforms[i], local);
            }
        }

        // --- スキニング行列（最終結果）の計算 ---
        for (size_t i = 0; i < mBones.size(); ++i) {
            XMMATRIX bind = XMLoadFloat4x4(&mBones[i].BindMatrix);
            XMMATRIX world = XMLoadFloat4x4(&outTransforms[i]);
            XMStoreFloat4x4(&outTransforms[i], bind * world);
        }
    }
}