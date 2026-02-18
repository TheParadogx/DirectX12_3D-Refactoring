#pragma once
#include<cstdint>
#include<Utility/Types/EcseTypes.hpp>
#include<Utility/Export/Export.hpp>

namespace Ecse::Graphics
{

	/// <summary>
	/// フォーマットのバッファサイズ用enum
	/// </summary>
	enum class eIndexBufferFormat : std::underlying_type_t<DXGI_FORMAT>
	{
		Uint32 = DXGI_FORMAT_R32_UINT,
		Uint16 = DXGI_FORMAT_R16_UINT,
	};

	/// <summary>
	/// インデックスバッファの管理をするクラス
	/// </summary>
	class ENGINE_API IndexBuffer
	{
	public:
		IndexBuffer();
		virtual ~IndexBuffer();

		// コピー禁止（GPUリソースを保持するため）
		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer& operator=(const IndexBuffer&) = delete;

		/// <summary>
		/// 作成
		/// </summary>
		/// <param name="Size">構造体のサイズ</param>
		/// <returns></returns>
		bool Create(size_t Size, eIndexBufferFormat Format);

		/// <summary>
		/// リソース解放
		/// </summary>
		void Release();

		/// <summary>
		/// 永続マッピングを高速更新
		/// </summary>
		/// <param name="SrcData"></param>
		/// <param name="Size"></param>
		/// <param name="Offset"></param>
		void Update(const void* SrcData, size_t Size, size_t Offset = 0);

		/// <summary>
		/// Viewセット
		/// </summary>
		void Set(ID3D12GraphicsCommandList* CmdList) const;

		/// <summary>
		/// 特定の範囲・フォーマットでViewセット
		/// </summary>
		/// <param name="Format"></param>
		/// <param name="Offset"></param>
		/// <param name="Size"></param>
		void Set(ID3D12GraphicsCommandList* CmdList, size_t Offset, size_t Size) const;

		/// <summary>
		/// バッファのサイズ
		/// </summary>
		/// <returns></returns>
		size_t GetSize() const;

	private:
		/// <summary>
		/// ビューをキャッシュ
		/// </summary>
		D3D12_INDEX_BUFFER_VIEW mBufferView;

		/// <summary>
		/// バッファのポインタ
		/// </summary>
		Resource mBufferResource;

		/// <summary>
		/// 確保したバッファのサイズ
		/// </summary>
		size_t mBufferSize;

		/// <summary>
		/// フォーマット
		/// </summary>
		eIndexBufferFormat mFormat;

		/// <summary>
		/// 永続マッピング用のポインタ
		/// </summary>
		void* mMapped;
	};



}