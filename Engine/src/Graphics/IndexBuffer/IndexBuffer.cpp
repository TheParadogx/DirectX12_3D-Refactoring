#include"pch.h"
#include<Graphics/IndexBuffer/IndexBuffer.hpp>
#include<Graphics/DX12/DX12.hpp>

namespace Ecse::Graphics
{
	IndexBuffer::IndexBuffer()
		:mBufferResource(nullptr)
		, mBufferView()
		, mBufferSize(0)
		, mMapped(nullptr)
	{
	}

	/// <summary>
	/// 作成
	/// </summary>
	/// <param name="Size">構造体のサイズ</param>
	/// <returns></returns>
	bool IndexBuffer::Create(size_t Size, eIndexBufferFormat Format)
	{
		mBufferSize = Size;
		auto device = System::ServiceLocator::Get<DX12>()->GetDevice();

		auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(mBufferSize);

		HRESULT hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&mBufferResource)
		);
		if (FAILED(hr))
		{
			ECSE_LOG(System::eLogLevel::Error, "Failed CreateIndexBuffer.");
			return false;
		}

		// 解放までUmmapしない
		CD3DX12_RANGE readRange(0, 0);
		mBufferResource->Map(0, &readRange, &mMapped);

		// デフォルトのビューを構成
		mBufferView.BufferLocation = mBufferResource->GetGPUVirtualAddress();
		mBufferView.SizeInBytes = static_cast<UINT>(mBufferSize);
		mBufferView.Format = static_cast<DXGI_FORMAT>(Format);

		mFormat = Format;

		return true;
	}

	/// <summary>
	/// リソース解放
	/// </summary>
	void IndexBuffer::Release()
	{
		if (mBufferResource != nullptr)
		{
			mBufferResource->Unmap(0, nullptr);
			mMapped = nullptr;
			mBufferResource.Reset();
		}
	}

	/// <summary>
	/// 永続マッピングを高速更新
	/// </summary>
	/// <param name="SrcData"></param>
	/// <param name="Size"></param>
	/// <param name="Offset"></param>
	void IndexBuffer::Update(const void* SrcData, size_t Size, size_t Offset)
	{
		if (mMapped == nullptr || (Offset + Size) > mBufferSize)
		{
			ECSE_LOG(System::eLogLevel::Warning, "range or not initialized.");
			return;
		}

		memcpy(static_cast<uint8_t*>(mMapped) + Offset, SrcData, Size);
	}


	/// <summary>
	/// Viewセット
	/// </summary>
	void IndexBuffer::Set(ID3D12GraphicsCommandList* CmdList) const
	{
		if (CmdList == nullptr) return;
		CmdList->IASetIndexBuffer(&mBufferView);
	}

	/// <summary>
	/// 特定の範囲・フォーマットでViewセット
	/// </summary>
	/// <param name="Format"></param>
	/// <param name="Offset"></param>
	/// <param name="Size"></param>
	void IndexBuffer::Set(ID3D12GraphicsCommandList* CmdList,size_t Offset, size_t Size) const
	{
		// 呼び出しごとに一時的にViewを作ってセット（特定範囲用）
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = mBufferView.BufferLocation + Offset;
		ibv.SizeInBytes = static_cast<UINT>(Size);
		ibv.Format = static_cast<DXGI_FORMAT>(mFormat);

		CmdList->IASetIndexBuffer(&ibv);
	}

	/// <summary>
	/// バッファのサイズ
	/// </summary>
	/// <returns></returns>
	size_t IndexBuffer::GetSize()const
	{
		return mBufferSize;
	}

}
