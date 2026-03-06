#include"pch.h"
#include<Graphics/FBX/Resource/FbxResource.hpp>
#include<Graphics/Texture/Texture.hpp>
#include<Graphics/Texture/Manager/TextureManager.hpp>
#include<Graphics/IndexBuffer/IndexBuffer.hpp>
#include<Graphics/VertexBuffer/VertexBuffer.hpp>
#include<Graphics/Data/GraphicsData.hpp>

using namespace DirectX;

namespace Ecse::Graphics
{

	FbxResource::FbxResource()
		:mVB(nullptr)
		,mIB(nullptr)
	{

	}
	FbxResource::~FbxResource()
	{
		Release();
	}

	/// <summary>
	/// リソースの解放
	/// </summary>
	void FbxResource::Release()
	{
		mMaterials.clear(); 
		mBones.clear(); 
		mAnimations.clear();
	}

	/// <summary>
	/// バッファのセット
	/// </summary>
	/// <param name="CmdList"></param>
	void FbxResource::SetBuffers(ID3D12GraphicsCommandList* CmdList) const
	{
		if (mVB) mVB->Set(CmdList);
		if (mIB) mIB->Set(CmdList);
	}

	std::span<const std::vector<DirectX::XMFLOAT4X4>> FbxResource::GetAnimationKeyFrames(const std::string& name) const
	{
		auto it = mAnimations.find(name);
		if (it != mAnimations.end())
		{
			return it->second.KeyFrame;
		}
		return std::span<const std::vector<DirectX::XMFLOAT4X4>>();
	}

	D3D12_VERTEX_BUFFER_VIEW FbxResource::GetVertexBufferView() const
	{
		if (mVB)
		{
			return mVB->GetView();
		}
		return {};
	}

	D3D12_INDEX_BUFFER_VIEW FbxResource::GetIndexBufferView() const
	{
		if (mIB)
		{
			return mIB->GetView();
		}
		return {};
	}

	/// <summary>
	/// FBXリソース作成
	/// </summary>
	/// <param name="FilePath">ファイルパス</param>
	/// <returns>true:成功</returns>
	bool FbxResource::Create(const std::filesystem::path& FilePath)
	{
		Release();
		return LoadModelData(FilePath);
	}

	/// <summary>
	/// アニメーションファイルの読込
	/// </summary>
	/// <param name="AnimationName">アニメーション名</param>
	/// <param name="AnimationPath">アニメーションのファイルパス</param>
	/// <returns>true:成功</returns>
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
		anim.KeyFrame.resize(fCount);
		for (auto& frame : anim.KeyFrame) {
			float time;
			ifs.read((char*)&time, 4);
			frame.resize(bCount);
			ifs.read((char*)frame.data(), 64 * bCount);
		}

		mAnimations[AnimationName] = std::move(anim);
		return true;

	}

	/// <summary>
	/// 専用binからfbxの情報を読み込む
	/// </summary>
	/// <param name="FilePath">バイナリのファイルパス</param>
	/// <returns>true:成功</returns>
	bool FbxResource::LoadModelData(const std::filesystem::path& FilePath)
	{
		std::ifstream ifs(FilePath, std::ios::binary);
		if (!ifs) return false;

		uint32_t head[5];
		ifs.read(reinterpret_cast<char*>(head), 20);
		if (head[0] != 0x4D4F444C) return false;

		uint32_t vCount = head[2];
		uint32_t iCount = head[3];
		uint32_t bCount = head[4];

		// 頂点
		std::vector<SkeletalMeshVertex> vertices(vCount);
		ifs.read((char*)vertices.data(), sizeof(SkeletalMeshVertex) * vCount);

		mVB = std::make_unique<VertexBuffer>();
		if(!mVB->Create(sizeof(SkeletalMeshVertex) * vCount, sizeof(SkeletalMeshVertex)))return false;
		mVB->Update(vertices.data(), sizeof(SkeletalMeshVertex) * vCount);

		// インデックス
		std::vector<uint32_t> indices(iCount);
		ifs.read((char*)indices.data(), sizeof(uint32_t) * iCount);

		mIB = std::make_unique<IndexBuffer>();
		if (!mIB->Create(sizeof(uint32_t) * iCount, eIndexBufferFormat::Uint32)) return false;
		mIB->Update(indices.data(), sizeof(uint32_t) * iCount);

		// サブメッシュ/マテリアル範囲
		uint32_t sCount = 0;
		ifs.read((char*)&sCount, 4);
		mMaterials.resize(sCount);
		for (auto& mat : mMaterials) {
			uint32_t matIdx;
			ifs.read((char*)&matIdx, 4);
			ifs.read((char*)&mat.StartIndex, 4);
			ifs.read((char*)&mat.IndexCount, 4);
		}

		// ボーン
		mBones.resize(bCount);
		for (auto& b : mBones) {
			uint32_t nLen;
			ifs.read((char*)&nLen, 4);
			b.Name.resize(nLen);
			ifs.read(&b.Name[0], nLen);
			ifs.read((char*)&b.ParentIndex, 4);
			// XMFLOAT4X4は16要素のfloat配列としてそのまま読める
			ifs.read((char*)&b.BindMatrix, sizeof(XMFLOAT4X4));
		}

		// マテリアル
		uint32_t mCount = 0;
		ifs.read((char*)&mCount, 4);
		std::filesystem::path baseDir = FilePath.parent_path();
		TextureManager* Manager = System::ServiceLocator::Get<TextureManager>();
		for (uint32_t i = 0; i < mCount; ++i) {
			uint32_t nL, tL;
			std::string mName, tPath;
			ifs.read((char*)&nL, 4); mName.resize(nL); ifs.read(&mName[0], nL);
			ifs.read((char*)&tL, 4); tPath.resize(tL); ifs.read(&tPath[0], tL);

			if (i < mMaterials.size()) {

				mMaterials[i].Name = mName;
				auto path = baseDir / tPath;
				mMaterials[i].Texture =
					Manager->GetOrLoad(path);
			}
		}

		return true;
	}


}
