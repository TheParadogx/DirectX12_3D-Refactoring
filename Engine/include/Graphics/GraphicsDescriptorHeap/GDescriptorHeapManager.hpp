#pragma once

#include<array>
#include<Utility/Types/EcseTypes.hpp>
#include<System/Service/ServiceProvider.hpp>

namespace Ecse::Graphics
{
	struct GDescritorHeapInfo;

	/// <summary>
	/// GDHの管理と空き領域の捜索
	/// </summary>
	class GDescriptorHeapManager : public System::ServiceProvider<GDescriptorHeapManager >
	{
		ECSE_SERVICE_ACCESS(GDescriptorHeapManager);

		/// <summary>
		/// 1領域の最大サイズ
		/// </summary>
		static constexpr int MAX_DESCRIPTOR = 512;
	public:

		/// <summary>
		/// 初期化
		/// </summary>
		/// <returns>true:成功</returns>
		bool Initialize();

		/// <summary>
		/// ヒープから使用領域の発行
		/// </summary>
		/// <param name="size"></param>
		/// <returns></returns>
		[[nodiscard]] GDescritorHeapInfo Issuance(uint32_t Size);

		/// <summary>
		/// 使用領域の破棄
		/// </summary>
		/// <param name="Info"></param>
		void Discard(GDescritorHeapInfo& Info);

		/// <summary>
		/// Cpuのハンドル取得
		/// </summary>
		/// <returns></returns>
		D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(const GDescritorHeapInfo& info) const;
		/// <summary>
		/// Gpuのハンドルの取得
		/// </summary>
		/// <returns></returns>
		D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(const GDescritorHeapInfo& info) const;
		/// <summary>
		/// ヒープ領域全体の取得
		/// </summary>
		/// <returns></returns>
		ID3D12DescriptorHeap* GetNativeHeap() const;

	private:
		/// <summary>
		/// 情報をセットに
		/// </summary>
		struct HandleInfo {
			D3D12_CPU_DESCRIPTOR_HANDLE cpu;
			D3D12_GPU_DESCRIPTOR_HANDLE gpu;
		};

		/// <summary>
		/// ヒープ領域
		/// </summary>
		Heap mHeap;

		/// <summary>
		/// 仕様可能なハンドルのコレクション
		/// </summary>
		std::array<HandleInfo, MAX_DESCRIPTOR> mHandles;

		/// <summary>
		/// ハンドルが使用中かどうか true:使用中
		/// </summary>
		std::array<bool, MAX_DESCRIPTOR> mIsUse;

		/// <summary>
		/// 今のサイズ
		/// </summary>
		uint32_t mDescriptorSize = 0;

		/// <summary>
		/// 検索操作で使用されるオフセットを保持
		/// </summary>
		int mSearchOffset = 0;
	};
}


