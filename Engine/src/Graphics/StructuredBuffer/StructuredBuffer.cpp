#include "pch.h"
#include<Graphics/StructuredBuffer/StructuredBuffer.hpp>
#include<Graphics/DX12/DX12.hpp>
#include<Graphics/GraphicsDescriptorHeap/GDescriptorHeapManager.hpp>

namespace Ecse::Graphics
{

	/// <summary>
	/// 現在のインデックスを取得するヘルパー
	/// dx12から取得する
	/// </summary>
	/// <returns></returns>
	uint32_t StructuredBuffer::GetCurrentIndex() const
	{
		return System::ServiceLocator::Get<DX12>()->GetCurrentFrameIndex();
	}
	StructuredBuffer::StructuredBuffer()
		:mElementSize(0)
		, mElementCount(0)
		, mTotalSize(0)
	{

	}
	StructuredBuffer::~StructuredBuffer()
	{
		Release();
	}

    /// <summary>
    /// 構造化バッファの作成
    /// </summary>
    /// <param name="ElementSize">構造体サイズ</param>
    /// <param name="ElementCount">数</param>
    /// <returns></returns>
	bool StructuredBuffer::Create(size_t ElementSize, uint32_t ElementCount)
	{
        Release();

        // 必要なものを取得
        auto dx12 = System::ServiceLocator::Get<DX12>();
        auto allocator = dx12->GetMAAllocator();
        auto device = dx12->GetDevice();
        auto heapManager = System::ServiceLocator::Get<GDescriptorHeapManager>();

        mElementSize = ElementSize;
        mElementCount = ElementCount;
        mTotalSize = ElementSize * ElementCount;

        auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(mTotalSize);
        D3D12MA::ALLOCATION_DESC allocDesc = { .HeapType = D3D12_HEAP_TYPE_UPLOAD };

        uint32_t frameCount = dx12->FRAME_COUNT;
        mFrames.resize(frameCount);

        for (uint32_t i = 0; i < frameCount; ++i) {
            // リソース作成
            allocator->CreateResource(&allocDesc, &resDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                &mFrames[i].Allocation, IID_PPV_ARGS(&mFrames[i].Resource));

            mFrames[i].Resource->Map(0, nullptr, &mFrames[i].MappedData);

            // 自前ヒープを作らず、マネージャーから1スロット確保
            mFrames[i].HeapInfo = heapManager->Issuance(1);
            if (!mFrames[i].HeapInfo.IsValid()) return false;

            // SRV設定
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_UNKNOWN;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Buffer.NumElements = ElementCount;
            srvDesc.Buffer.StructureByteStride = (UINT)ElementSize;

            // マネージャーが管理する共通ヒープの場所にSRVを書き込む
            device->CreateShaderResourceView(
                mFrames[i].Resource.Get(),
                &srvDesc,
                heapManager->GetCpuHandle(mFrames[i].HeapInfo)
            );
        }
        return true;
    }

    /// <summary>
    /// リソース解放
    /// </summary>
    void StructuredBuffer::Release()
    {
        auto heapManager = System::ServiceLocator::Get<GDescriptorHeapManager>();
        for (auto& frame : mFrames) {
            if (frame.Resource && frame.MappedData) {
                frame.Resource->Unmap(0, nullptr);
            }
            // スロットを返す
            if (frame.HeapInfo.IsValid()) {
                heapManager->Discard(frame.HeapInfo);
            }
            frame.Allocation.Reset();
            frame.Resource.Reset();
        }
    }

    /// <summary>
    /// データをGPUに転送
    /// </summary>
    /// <param name="SrcData"></param>
    /// <param name="Size"></param>
    void StructuredBuffer::Update(const void* srcData, size_t size)
    {
        if (!srcData || mFrames.empty()) return;

        // 今のフレームのリソース取得
        auto& frame = mFrames[GetCurrentIndex()];

        // 念のためサイズチェック
        size_t copySize = (size < mTotalSize) ? size : mTotalSize;
        memcpy(frame.MappedData, srcData, copySize);
    }

    /// <summary>
    /// DescriptorTable用のGPUハンドル取得 
    /// </summary>
    /// <returns></returns>
    D3D12_GPU_DESCRIPTOR_HANDLE StructuredBuffer::GetGpuHandle()const
    {
        auto& frame = mFrames[GetCurrentIndex()];
        // マネージャーから、このフレームに割り当てられたGPUハンドルを取得
        return System::ServiceLocator::Get<GDescriptorHeapManager>()->GetGpuHandle(frame.HeapInfo);
    }
}
