#include "pch.h"
#include<Graphics/GraphicsDescriptorHeap/GDescriptorHeapManager.hpp>
#include<Graphics/GraphicsDescriptorHeap/GDescriptorHeapInfo.hpp>

#include<Graphics/DX12/DX12.hpp>


/// <summary>
/// 将来的には一時と永続で領域を分けようかな
/// </summary>

namespace Ecse::Graphics
{

	/// <summary>
	/// 初期化
	/// </summary>
	/// <returns>true:成功</returns>
	bool GDescriptorHeapManager::Initialize()
	{
		auto dx12 = System::ServiceLocator::Get<DX12>();
		auto device = dx12->GetDevice();

		// ディスクリプタヒープの作成
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = MAX_DESCRIPTOR;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // シェーダーから参照可能にする
		desc.NodeMask = 0;
		HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mHeap));
		if (FAILED(hr))
		{
			ECSE_LOG(System::ELogLevel::Fatal, "Failed CreateDescriptorHeap.");
			return false;
		}

		//	各ハンドルのインクリメントサイズと開始位置を取得
		mDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		auto cpuStart = mHeap->GetCPUDescriptorHandleForHeapStart();
		auto gpuStart = mHeap->GetGPUDescriptorHandleForHeapStart();

		//	全ハンドルのテーブルを事前計算
		for (int i = 0; i < MAX_DESCRIPTOR; ++i) {
			mHandles[i].cpu.ptr = cpuStart.ptr + (static_cast<SIZE_T>(i) * mDescriptorSize);
			mHandles[i].gpu.ptr = gpuStart.ptr + (static_cast<SIZE_T>(i) * mDescriptorSize);
			mIsUse[i] = false;
		}
		mSearchOffset = 0;
		ECSE_LOG(System::ELogLevel::Log, "DescriptorHeapManager: Initialized with " + std::to_string(MAX_DESCRIPTOR) + " slots.");


		return true;

	}
		 
	/// <summary>
	/// ヒープから使用領域の発行
	/// </summary>
	/// <param name="size"></param>
	/// <returns></returns>
    GDescritorHeapInfo GDescriptorHeapManager::Issuance(uint32_t Size)
    {

		for (int count = 0; count < MAX_DESCRIPTOR; ) {
			int current = (mSearchOffset + count) % MAX_DESCRIPTOR;

			// ヒープ末尾をまたぐ確保は不可
			if (current + (int)Size > MAX_DESCRIPTOR) {
				count += (MAX_DESCRIPTOR - current); // 先頭までジャンプ
				continue;
			}

			bool isFree = true;
			int conflictIndexInRequest = -1;

			for (uint32_t s = 0; s < Size; ++s) {
				if (mIsUse[current + s]) {
					isFree = false;
					conflictIndexInRequest = (int)s; // 何番目のスロットで衝突したか記録
					break;
				}
			}

			if (isFree) {
				// 確保成功
				for (uint32_t i = 0; i < Size; ++i) mIsUse[current + i] = true;
				mSearchOffset = (current + Size) % MAX_DESCRIPTOR;
				return { current, (int)Size };
			}

			// 【ここが重要】衝突した場所の「次」まで count を進める
			// 0番目から探して2番目がダメなら、次は3番目から探せば良い
			count += (conflictIndexInRequest + 1);
		}

		ECSE_LOG(System::ELogLevel::Error, "DescriptorHeapManager: Out of descriptors!");
		return { -1, 0 };
	}

	/// <summary>
	/// 使用領域の破棄
	/// </summary>
	/// <param name="Info"></param>
	void GDescriptorHeapManager::Discard(GDescritorHeapInfo& Info)
	{
		//	インデックスが有効かどうか
		if (Info.IsValid() == false) return;

		//	指定された範囲のフラグを未使用に戻す
		const int lastIndex = Info.Index + Info.Size;
		//	ヒープの最大数を超えないためのカード
		const int actualEnd = (lastIndex > MAX_DESCRIPTOR) ? MAX_DESCRIPTOR : lastIndex;
		for (int i = Info.Index; i < actualEnd; ++i)
		{
			mIsUse[i] = false;
		}

		//	使い終わったInfoの初期化
		Info.Index = -1;
		Info.Size = 0;
	}

	/// <summary>
	/// Cpuのハンドル取得
	/// </summary>
	/// <returns></returns>
	D3D12_CPU_DESCRIPTOR_HANDLE GDescriptorHeapManager::GetCpuHandle(const GDescritorHeapInfo& info) const
	{
		// infoが無効、または範囲外の場合は0を返してクラッシュを防ぐ
		if (!info.IsValid() || info.Index >= MAX_DESCRIPTOR)
		{
			return {0};
		}

		// 事前計算済みの配列から取得
		return mHandles[info.Index].cpu;
	}

	/// <summary>
	/// Gpuのハンドルの取得
	/// </summary>
	/// <returns></returns>
	D3D12_GPU_DESCRIPTOR_HANDLE GDescriptorHeapManager::GetGpuHandle(const GDescritorHeapInfo& info) const
	{
		// infoが無効、または範囲外の場合は0を返してクラッシュを防ぐ
		if (!info.IsValid() || info.Index >= MAX_DESCRIPTOR)
		{
			return { 0 };
		}

		// 事前計算済みの配列から取得
		return mHandles[info.Index].gpu;
	}

	/// <summary>
	/// ヒープ領域全体の取得
	/// </summary>
	/// <returns></returns>
	ID3D12DescriptorHeap* GDescriptorHeapManager::GetNativeHeap() const
	{
		return mHeap.Get();
	}

}
