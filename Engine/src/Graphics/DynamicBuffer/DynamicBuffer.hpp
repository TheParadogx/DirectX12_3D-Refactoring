#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <algorithm>

namespace Ecse::Graphics
{
    class DynamicBuffer {
    public:

        static constexpr size_t CB_ALIGN = 256;
        static constexpr size_t MIN_BUFFER_SIZE = 1024;

        DynamicBuffer(ID3D12Device* device, size_t sizePerFrame, uint32_t frameCount);
        ~DynamicBuffer();

        DynamicBuffer(const DynamicBuffer&) = delete;
        DynamicBuffer& operator=(const DynamicBuffer&) = delete;

        /// <summary>
        /// リソースのリセット
        /// </summary>
        /// <param name="frameIndex"></param>
        void Reset(uint32_t frameIndex);

        /// <summary>
		/// 単一の構造体をアップロードするヘルパー
        /// </summary>
        template<typename T>
        D3D12_GPU_VIRTUAL_ADDRESS Upload(uint32_t frameIndex, const T& data)
        {
            static_assert(std::is_trivially_copyable_v<T>,
                "DynamicBuffer Upload requires trivially copyable type");

            return AllocateAndCopy(frameIndex, &data, sizeof(T));
        }

        /// <summary>
		/// 配列をアップロードするヘルパー
        /// </summary>
        /// <returns></returns>
        template<typename T>
        D3D12_GPU_VIRTUAL_ADDRESS UploadArray(
            uint32_t frameIndex,
            const T* data,
            size_t elementCount)
        {
            static_assert(std::is_trivially_copyable_v<T>,
                "DynamicBuffer UploadArray requires trivially copyable type");

            if (!data)
                throw std::invalid_argument("DynamicBuffer UploadArray: data is null");

            if (elementCount == 0)
                throw std::invalid_argument("DynamicBuffer UploadArray: elementCount is zero");

            if (elementCount > (SIZE_MAX / sizeof(T)))
                throw std::runtime_error("DynamicBuffer UploadArray: size overflow");

            return AllocateAndCopy(frameIndex, data, sizeof(T) * elementCount);
        }

    private:
        /// <summary>
		/// 1フレーム分のリソースと管理情報
        /// </summary>
        struct FrameResource
        {
            Microsoft::WRL::ComPtr<ID3D12Resource> resource;

            uint8_t* mappedData = nullptr;

            size_t capacity = 0;
            size_t offset = 0;

            D3D12_GPU_VIRTUAL_ADDRESS gpuBase = 0;
        };

    private:
        /// <summary>
		/// サイズをCBVのアライメントに合わせる
        /// </summary>
        /// <param name="size"></param>
        /// <returns></returns>
        size_t Align(size_t size) const
        {
            return (size + CB_ALIGN - 1) & ~(CB_ALIGN - 1);
        }

        /// <summary>
		/// フレームのリソースを必要なサイズに合わせて拡張する
        /// </summary>
        void GrowFrame(FrameResource& frame, size_t requiredSize);

        /// <summary>
		/// フレームのリソースにデータをコピーして、GPU仮想アドレスを返す
        /// </summary>
        D3D12_GPU_VIRTUAL_ADDRESS AllocateAndCopy(
            uint32_t frameIndex,
            const void* data,
            size_t size);

    private:
        /// <summary>
		/// デバイスはリソース作成のために保持するが、DynamicBuffer自体はデバイスに依存しない
        /// </summary>
        ID3D12Device* mDevice = nullptr;

        /// <summary>
		/// フレームごとのリソースと管理情報の配列
        /// </summary>
        std::vector<FrameResource> mFrames;

        /// <summary>
		/// フレームごとのリソースの初期サイズ
        /// </summary>
        size_t mInitialSize = 0;
    };
}

