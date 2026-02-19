#pragma once
#include<Utility/Types/EcseTypes.hpp>
#include<Graphics/GraphicsDescriptorHeap/GDescriptorHeap.hpp>
#include<vector>
#include<Utility/Export/Export.hpp>

namespace Ecse::Graphics
{
	/// <summary>
	/// 定数バッファ管理（シェーダーなど）
	/// </summary>
	class ENGINE_API ConstantBuffer
	{
	public:
		ConstantBuffer();
		virtual ~ConstantBuffer();

		// ポインタ持っているのでコピーされると困るから禁止
		ConstantBuffer(const ConstantBuffer&) = delete;
		ConstantBuffer& operator=(const ConstantBuffer&) = delete;

		/// <summary>
		/// 定数バッファの作成
		/// </summary>
		/// <param name="Size">構造体の実サイズ</param>
		/// <returns>成功ならtrue</returns>
		bool Create(size_t Size);

		/// <summary>
		/// リソースの解放
		/// </summary>
		void Release();

		/// <summary>
		/// CPUからGPUへデータを転送
		/// </summary>
		/// <param name="SrcData">転送元構造体のポインタ</param>
		void Update(const void* SrcData);

		/// <summary>
		/// GPUハンドルの取得
		/// </summary>
		D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const;

		/// <summary>
		/// Map済みのポインタからオフセット分ずらしたアドレスの取得
		/// </summary>
		/// <param name="Offset"></param>
		/// <returns></returns>
		void* GetMappedPtr(size_t Offset = 0);

		/// <summary>
		/// 先頭からオフセット分ずらした仮想アドレスを返す
		/// </summary>
		/// <param name="offset"></param>
		/// <returns></returns>
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(size_t Offset = 0);

	private:
		// フレームごとのリソースセット
		struct FrameResource {
			Resource Resource = nullptr;
			MAAllocation Allocation = nullptr;
			GDescriptorHeap Heap;
			void* MappedData = nullptr;
		};
		std::vector<FrameResource> mFrames;
		size_t mActualSize = 0;
		size_t mAlignedSize = 0;
	};
}

