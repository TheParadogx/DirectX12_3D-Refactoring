#include "pch.h"
#include<Graphics/GraphicsDescriptorHeap/GDescriptorHeap.hpp>
#include<Graphics/GraphicsDescriptorHeap/GDescriptorHeapManager.hpp>

namespace Ecse::Graphics
{
	GDescriptorHeap::GDescriptorHeap()
		:mHeapInfo()
	{

	}
	GDescriptorHeap::~GDescriptorHeap()
	{
		this->Release();
	}


	/// <summary>
	/// ヒープ領域の確保
	/// </summary>
	/// <param name="Size">確保するスロット数</param>
	/// <returns>成功したらtrue</returns>
	bool GDescriptorHeap::Create(uint32_t Size)
	{
		// 既に確保済みなら一度解放
		if (mHeapInfo.IsValid()) {
			Release();
		}

		auto manager = System::ServiceLocator::Get<GDescriptorHeapManager>();
		if (manager == nullptr)
		{
			ECSE_LOG(System::ELogLevel::Fatal, "Failed GetGDescriptorHeapManager");
			return false;
		}

		//	マネージャーから枠を貸してもらう
		mHeapInfo = manager->Issuance(Size);

		return mHeapInfo.IsValid();
	}

	/// <summary>
	/// 明示的な解放（デストラクタでも呼びます）
	/// </summary>
	void GDescriptorHeap::Release()
	{
		if (mHeapInfo.IsValid())
		{
			auto manager = System::ServiceLocator::Get<GDescriptorHeapManager>();
			if (manager != nullptr)
			{
				manager->Discard(mHeapInfo);
			}
		}
	}

	/// <summary>
	/// 貸し出されているCPUのハンドル取得
	/// </summary>
	/// <returns></returns>
	D3D12_CPU_DESCRIPTOR_HANDLE GDescriptorHeap::GetCpuHandle() const
	{
		auto manager = System::ServiceLocator::Get<GDescriptorHeapManager>();
		if (manager != nullptr)
		{
			return manager->GetCpuHandle(mHeapInfo);
		}

		return D3D12_CPU_DESCRIPTOR_HANDLE();
	}

	/// <summary>
	/// 貸し出されているGPUのハンドル取得
	/// </summary>
	/// <returns></returns>
	D3D12_GPU_DESCRIPTOR_HANDLE GDescriptorHeap::GetGpuHandle() const
	{

		auto manager = System::ServiceLocator::Get<GDescriptorHeapManager>();
		if (manager != nullptr)
		{
			return manager->GetGpuHandle(mHeapInfo);
		}

		return D3D12_GPU_DESCRIPTOR_HANDLE();
	}
}
