#pragma once
#include<Utility/Export/Export.hpp>
#include<Utility/Types/EcseTypes.hpp>
#include<DirectXMath.h>

#include<string>
#include<vector>
#include<filesystem>
#include<span>
#include<memory>
#include<unordered_map>

namespace Ecse::Graphics
{
	class Texture;
	class IndexBuffer;
	class VertexBuffer;

	/// <summary>
	/// Fbxの共通化するリソース管理
	/// </summary>
	class ENGINE_API FbxResource
	{
	public:
		/// <summary>
		/// マテリアル・描画範囲情報
		/// </summary>
		struct Material
		{
			std::string Name;
			Texture* Texture;
			uint32_t IndexCount = 0;
			uint32_t StartIndex = 0;
		};

		/// <summary>
		/// ボーン・スキニング用情報
		/// </summary>
		struct BoneInfo
		{
			std::string Name;
			int ParentIndex;
			// 逆バインドポーズ行列（計算用にXMFLOAT4X4）
			DirectX::XMFLOAT4X4 BindMatrix;
		};

		/// <summary>
		/// アニメーションデータ
		/// </summary>
		struct Animation
		{
			int NumFrame;
			// KeyFrame[フレームIndex][ボーンIndex]
			std::vector<std::vector<DirectX::XMFLOAT4X4>> KeyFrame;
		};

	public:
		FbxResource();
		virtual ~FbxResource();

		/// <summary>
		/// FBXリソース作成
		/// </summary>
		/// <param name="FilePath">ファイルパス</param>
		/// <returns>true:成功</returns>
		bool Create(const std::filesystem::path& FilePath);

		/// <summary>
		/// アニメーションファイルの読込
		/// </summary>
		/// <param name="AnimationName">アニメーション名</param>
		/// <param name="AnimationPath">アニメーションのファイルパス</param>
		/// <returns>true:成功</returns>
		bool LoadAnimation(const std::string& AnimationName, const std::filesystem::path& AnimationPath);

		/// <summary>
		/// リソースの解放
		/// </summary>
		void Release();

		/// <summary>
		/// バッファのセット
		/// </summary>
		/// <param name="CmdList"></param>
		void SetBuffers(ID3D12GraphicsCommandList* CmdList) const;

		/*
		* ゲッター
		* 各情報へのアクセス
		*/
		std::span<const Material> GetMaterials() const { return mMaterials; }
		std::span<const BoneInfo> GetBones() const { return mBones; }
		const std::unordered_map<std::string, Animation>& GetAnimations() const { return mAnimations; }

		std::span<const std::vector<DirectX::XMFLOAT4X4>> GetAnimationKeyFrames(const std::string& name) const;

		D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const;
		D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const;
	private:
		/// <summary>
		/// 専用binからfbxの情報を読み込む
		/// </summary>
		/// <param name="FilePath">バイナリのファイルパス</param>
		/// <returns>true:成功</returns>
		bool LoadModelData(const std::filesystem::path& FilePath);

	private:
		/// <summary>
		/// マテリアル
		/// </summary>
		std::vector<Material> mMaterials;
		/// <summary>
		/// ボーン情報
		/// </summary>
		std::vector<BoneInfo> mBones;
		/// <summary>
		/// アニメーションのマップ
		/// </summary>
		std::unordered_map<std::string, Animation> mAnimations;
		/// <summary>
		/// ボーン名とインデックスのバインド
		/// </summary>
		std::unordered_map<std::string, int> mBoneIndexMap;

		/// <summary>
		/// 頂点バッファ　
		/// </summary>
		std::unique_ptr<VertexBuffer> mVB;
		/// <summary>
		/// インデックスバッファ
		/// </summary>
		std::unique_ptr<IndexBuffer> mIB;
	};
}