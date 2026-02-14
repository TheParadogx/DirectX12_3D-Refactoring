#pragma once
#include<Graphics/GraphicsDescriptorHeap/GDescriptorHeapInfo.hpp>
#include<cstdint>
#include <d3d12.h>

namespace Ecse::Graphics
{
	/// <summary>
	/// ディスクリプタヒープの1枠（または連続した枠）を管理するRAIIクラス
	/// </summary>
	class GDescriptorHeap
	{
	public:
		GDescriptorHeap();
		virtual ~GDescriptorHeap();

		// 二重解放を防ぐためにコピー禁止
		GDescriptorHeap(const GDescriptorHeap&) = delete;
		GDescriptorHeap& operator=(const GDescriptorHeap&) = delete;

		/// <summary>
		/// ヒープ領域の確保
		/// </summary>
		/// <param name="Size">確保するスロット数</param>
		/// <returns>成功したらtrue</returns>
		bool Create(uint32_t Size = 1);

		/// <summary>
		/// 明示的な解放（デストラクタでも呼びます）
		/// </summary>
		void Release();

		/// <summary>
		/// 貸し出されているCPUのハンドル取得
		/// </summary>
		/// <returns></returns>
		D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() const;

		/// <summary>
		/// 貸し出されているGPUのハンドル取得
		/// </summary>
		/// <returns></returns>
		D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const;

		/// <summary>
		/// 有効かどうか
		/// </summary>
		/// <returns></returns>
		bool IsValid() const;
	private:
		/// <summary>
		/// ハンドルのアクセス用
		/// </summary>
		GDescritorHeapInfo mHeapInfo;
	};
}