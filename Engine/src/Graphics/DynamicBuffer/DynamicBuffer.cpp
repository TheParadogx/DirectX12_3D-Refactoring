#include "pch.h"
#include "DynamicBuffer.hpp"

namespace Ecse::Graphics
{
    DynamicBuffer::DynamicBuffer(
        ID3D12Device* device,
        size_t sizePerFrame,
        uint32_t frameCount)
    {
        if (!device)
            throw std::invalid_argument("DynamicBuffer: device is null");

        if (frameCount == 0)
            throw std::invalid_argument("DynamicBuffer: frameCount must be > 0");

        mDevice = device;

        size_t rawSize = (std::max)(sizePerFrame, MIN_BUFFER_SIZE);
        mInitialSize = Align(rawSize);

        mFrames.resize(frameCount);

        for (uint32_t i = 0; i < frameCount; ++i)
        {
            GrowFrame(mFrames[i], mInitialSize);
        }
    }

    DynamicBuffer::~DynamicBuffer()
    {
        for (auto& frame : mFrames)
        {
            if (frame.resource && frame.mappedData)
            {
                frame.resource->Unmap(0, nullptr);
            }
        }
    }

    void DynamicBuffer::Reset(uint32_t frameIndex)
    {
        if (frameIndex >= mFrames.size())
            throw std::out_of_range("DynamicBuffer Reset invalid frame index");

        mFrames[frameIndex].offset = 0;
    }

    void DynamicBuffer::GrowFrame(FrameResource& frame, size_t requiredSize)
    {
        size_t newSize = Align(requiredSize);

        Resource newResource;

        auto desc = CD3DX12_RESOURCE_DESC::Buffer(newSize);
        auto heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

        if (FAILED(mDevice->CreateCommittedResource(
            &heap,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&newResource))))
        {
            throw std::runtime_error("DynamicBuffer: CreateCommittedResource failed");
        }

        uint8_t* mapped = nullptr;
        D3D12_RANGE readRange{ 0,0 };

        if (FAILED(newResource->Map(
            0,
            &readRange,
            reinterpret_cast<void**>(&mapped))))
        {
            throw std::runtime_error("DynamicBuffer: Map failed");
        }

        // 旧データコピー
        if (frame.mappedData && frame.offset > 0)
        {
            std::memcpy(mapped, frame.mappedData, frame.offset);
        }

        frame.resource = newResource;
        frame.mappedData = mapped;
        frame.capacity = newSize;
        frame.gpuBase = newResource->GetGPUVirtualAddress();
    }

    D3D12_GPU_VIRTUAL_ADDRESS DynamicBuffer::AllocateAndCopy(
        uint32_t frameIndex,
        const void* data,
        size_t size)
    {
        if (frameIndex >= mFrames.size())
            throw std::out_of_range("DynamicBuffer invalid frame index");

        if (!data || size == 0)
            throw std::invalid_argument("DynamicBuffer invalid data");

        auto& frame = mFrames[frameIndex];

        size_t alignedSize = Align(size);

        if (frame.offset + alignedSize > frame.capacity)
        {
            size_t required = frame.offset + alignedSize;

            size_t newSize = (std::max)(required, frame.capacity * 2);

            GrowFrame(frame, newSize);
        }

        uint8_t* dest = frame.mappedData + frame.offset;

        std::memcpy(dest, data, size);

        D3D12_GPU_VIRTUAL_ADDRESS address =
            frame.gpuBase + frame.offset;

        frame.offset += alignedSize;

        return address;
    }
}