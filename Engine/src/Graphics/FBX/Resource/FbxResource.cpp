#include"pch.h"
#include<Graphics/FBX/Resource/FbxResource.hpp>
#include<Graphics/Texture/Texture.hpp>

using namespace DirectX;

namespace Ecse::Graphics
{
	/// <summary>
	/// FBXの頂点構造体
	/// </summary>
	struct SkeletalMeshVertex {
		XMFLOAT3 Position;
		XMFLOAT3 Normal;
		XMFLOAT2 UV;
		XMFLOAT3 Tangent;
		int32_t  BoneIndices[4];
		float    BoneWeights[4];
	};

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
		mVB.Reset(); 
		mIB.Reset();
		mMaterials.clear(); 
		mBones.clear(); 
		mAnimations.clear();
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

		return true;
	}

	/// <summary>
	/// 専用binからfbxの情報を読み込む
	/// </summary>
	/// <param name="FilePath">バイナリのファイルパス</param>
	/// <returns>true:成功</returns>
	bool FbxResource::LoadModelData(const std::filesystem::path& FilePath)
	{
		return true;
	}


}
