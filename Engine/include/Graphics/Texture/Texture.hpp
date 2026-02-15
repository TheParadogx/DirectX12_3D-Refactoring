#pragma once
#include<Graphics/GraphicsDescriptorHeap/GDescriptorHeapInfo.hpp>
#include<Utility/Types/EcseTypes.hpp>
#include<filesystem>

namespace Ecse::Graphics
{
	/// <summary>
	/// テクスチャのリソースのクラス
	/// </summary>
	class Texture
	{
	public:
		Texture();
		virtual ~Texture();

		/// <summary>
		/// リソースの作成
		/// </summary>
		/// <param name="FilePath">画像のファイルパス</param>
		/// <returns>true:成功</returns>
		bool Create(const std::filesystem::path& FilePath);
		/// <summary>
		/// リソースの解放
		/// </summary>
		void Release();

		/// <summary>
		/// 割り当てられたインデックス
		/// </summary>
		/// <returns></returns>
		uint32_t GetDescriptorIndex() const;

		/// <summary>
		/// 割り当てられたGpuハンドルの取得
		/// </summary>
		/// <returns></returns>
		D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const;

		/// <summary>
		/// テクスチャの幅
		/// </summary>
		/// <returns></returns>
		float GetWidth()const;
		/// <summary>
		/// テクスチャの高さ
		/// </summary>
		/// <returns></returns>
		float GetHeight()const;

		/// <summary>
		/// リソースの取得
		/// </summary>
		/// <returns></returns>
		ID3D12Resource* GetResource() const;
	private:
		/// <summary>
		/// 割り当てられたヒープの情報
		/// </summary>
		GDescritorHeapInfo mHeapInfo;

		/// <summary>
		/// リソース
		/// </summary>
		Resource mReource;

		/// <summary>
		/// テクスチャ横幅
		/// </summary>
		float mWidth;
		/// <summary>
		/// テクスチャ縦幅
		/// </summary>
		float mHeight;


	};
}

