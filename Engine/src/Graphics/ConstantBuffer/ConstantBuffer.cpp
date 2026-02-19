#include "pch.h"
#include<Graphics/ConstantBuffer/ConstantBuffer.hpp>
#include<Graphics/DX12/DX12.hpp>

namespace Ecse::Graphics
{
	ConstantBuffer::ConstantBuffer()
		: mActualSize(0)
		, mAlignedSize(0)
	{
	}

	ConstantBuffer::~ConstantBuffer()
	{
		Release();
	}

	/// <summary>
	/// 定数バッファの作成
	/// </summary>
	/// <param name="Size">構造体の実サイズ</param>
	/// <returns>成功ならtrue</returns>
	bool ConstantBuffer::Create(size_t Size)
	{
		//	リークしないように既にあるときは解放してから
		Release();

		if (Size <= 0) return false;

		auto dx12 = System::ServiceLocator::Get<DX12>();
		auto allocator = dx12->GetMAAllocator();
		auto device = dx12->GetDevice();

		//	フレーム数作成
		uint32_t frameCount = dx12->FRAME_COUNT;
		mFrames.clear();
		mFrames.reserve(frameCount);
		for (uint32_t i = 0; i < frameCount; ++i) {
			mFrames.emplace_back();
		}

		mActualSize = Size;
		mAlignedSize = (Size + 0xff) & ~0xff; //256境界に

		// リソース作成用のデスク作成
		auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(mAlignedSize);
		D3D12MA::ALLOCATION_DESC allocDesc = {};
		allocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

		for (uint32_t i = 0; i < frameCount; ++i) {
			// リソース作成
			HRESULT hr = allocator->CreateResource(&allocDesc, &resDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
				&mFrames[i].Allocation, IID_PPV_ARGS(&mFrames[i].Resource));
			if (FAILED(hr))
			{
				return false;
			}

			// Map時のRange指定
			CD3DX12_RANGE readRange(0, 0);
			hr = mFrames[i].Resource->Map(0, &readRange, &mFrames[i].MappedData);
			if (FAILED(hr))
			{
				return false;
			}

			// デスクリプタ確保
			if (mFrames[i].Heap.Create(1) == false)
			{
				return false;
			}

			// CBV作成
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = mFrames[i].Resource->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = static_cast<UINT>(mAlignedSize);
			device->CreateConstantBufferView(&cbvDesc, mFrames[i].Heap.GetCpuHandle());
		}


		return true;
	}

	/// <summary>
	/// リソースの解放
	/// </summary>
	void ConstantBuffer::Release()
	{
		for (auto& frame : mFrames) {
			if (frame.MappedData) {
				frame.Resource->Unmap(0, nullptr);
				frame.MappedData = nullptr;
			}
			frame.Allocation.Reset();
			frame.Resource.Reset();
			frame.Heap.Release();
		}
		mFrames.clear();
		mActualSize = 0;
		mAlignedSize = 0;
	}

	/// <summary>
	/// CPUからGPUへデータを転送
	/// </summary>
	/// <param name="SrcData">転送元構造体のポインタ</param>
	void ConstantBuffer::Update(const void* SrcData)
	{
		if (SrcData == nullptr || mFrames.empty()) return;

		// 現在のフレーム用バッファに安全に書き込み
		uint32_t index = System::ServiceLocator::Get<DX12>()->GetCurrentFrameIndex();
		memcpy(mFrames[index].MappedData, SrcData, mActualSize);
	}

	/// <summary>
	/// GPUハンドルの取得
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE ConstantBuffer::GetGpuHandle()const
	{
		uint32_t index = System::ServiceLocator::Get<DX12>()->GetCurrentFrameIndex();
		return mFrames[index].Heap.GetGpuHandle();
	}


	/// <summary>
	/// Map済みのポインタからオフセット分ずらしたアドレスの取得
	/// </summary>
	/// <param name="Offset"></param>
	/// <returns></returns>
	void* ConstantBuffer::GetMappedPtr(size_t Offset)
	{
		uint32_t index = System::ServiceLocator::Get<DX12>()->GetCurrentFrameIndex();
		// Map済みポインタから、指定されたオフセット分ずらしたアドレスを返す
		return static_cast<uint8_t*>(mFrames[index].MappedData) + Offset;
	}

	/// <summary>
	/// 先頭からオフセット分ずらした仮想アドレスを返す
	/// </summary>
	/// <param name="offset"></param>
	/// <returns></returns>
	D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetGPUAddress(size_t Offset)
	{
		uint32_t index = System::ServiceLocator::Get<DX12>()->GetCurrentFrameIndex();
		// リソースの先頭アドレス + オフセットを返す
		return mFrames[index].Resource->GetGPUVirtualAddress() + Offset;
	}

}

