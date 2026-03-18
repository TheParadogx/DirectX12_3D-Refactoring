#include "pch.h"
#include<Graphics/Texture/Texture.hpp>
#include<System/Service/ServiceLocator.hpp>
#include<Graphics/DX12/DX12.hpp>
#include<Graphics/GraphicsDescriptorHeap/GDescriptorHeapManager.hpp>

namespace fs = ::std::filesystem;
namespace DirectXTex = ::DirectX;

namespace Ecse::Graphics
{

	Texture::Texture()
		:mHeapInfo()
		, mResource(nullptr)
		,mAllocation(nullptr)
		, mWidth()
		, mHeight()
	{

	}

	Texture::~Texture()
	{
		Release();
	}


	/// <summary>
	/// リソースの作成
	/// </summary>
	/// <param name="FilePath">画像のファイルパス</param>
	/// <returns>true:成功</returns>
	bool Texture::Create(const std::filesystem::path& FilePath)
	{
		//	ファイルの存在判定
		if (fs::exists(FilePath) == false || fs::is_regular_file(FilePath) == false)
		{
			ECSE_LOG(System::eLogLevel::Error, "NotExists Texture. : " + FilePath.string());
			return false;
		}
		
		//	拡張子
		const fs::path ext = FilePath.extension();	
		//	ファイルパス
		std::wstring path = FilePath.wstring();
		HRESULT hr = S_FALSE;
		//	テクスチャのメタデータ
		DirectX::TexMetadata metaData = {};
		DirectX::ScratchImage scratchImage = {};

		//	大文字、小文字を無視して判定をする。
		if (_wcsicmp(ext.c_str(), L".dds") == 0)
		{
			hr = DirectXTex::LoadFromDDSFile(
				path.c_str(),
				DirectXTex::DDS_FLAGS_NONE,
				&metaData,
				scratchImage);
		}
		else if (_wcsicmp(ext.c_str(), L".tga") == 0)
		{
			hr = DirectXTex::LoadFromTGAFile(
				path.c_str(),
				DirectXTex::TGA_FLAGS_NONE,
				&metaData,
				scratchImage);
		}
		else if (_wcsicmp(ext.c_str(), L".hdr") == 0)
		{
			hr = DirectXTex::LoadFromHDRFile(
				path.c_str(),
				&metaData,
				scratchImage);
		}
		else
		{
			hr = DirectXTex::LoadFromWICFile(
				path.c_str(),
				DirectXTex::WIC_FLAGS_NONE,
				&metaData,
				scratchImage);

		}
		if (FAILED(hr))
		{
			ECSE_LOG(System::eLogLevel::Error, "Fialed LoadTexture.");
			return false;
		}

		Graphics::DX12* dx12 = System::ServiceLocator::Get<Graphics::DX12>();
		auto device = dx12->GetDevice();
		auto allocator = dx12->GetMAAllocator();
		auto cmdList = dx12->GetCommandList();

		// テクスチャリソース作成 GPU専用メモリでね。
		// metaDataから変換してデスクの作成
		D3D12_RESOURCE_DESC resDesc = {};
		resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metaData.dimension);
		resDesc.Format = metaData.format;
		resDesc.Width = static_cast<UINT64>(metaData.width);
		resDesc.Height = static_cast<UINT>(metaData.height);
		resDesc.DepthOrArraySize = static_cast<UINT16>(metaData.arraySize);
		resDesc.MipLevels = static_cast<UINT16>(metaData.mipLevels);
		resDesc.SampleDesc.Count = 1;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12MA::ALLOCATION_DESC texAllocDesc = {};
		texAllocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT; // VRAMへ

		// リソース作成
		hr = allocator->CreateResource(&texAllocDesc, &resDesc, D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr, &mAllocation, IID_PPV_ARGS(&mResource));
		if (FAILED(hr))
		{
			ECSE_LOG(System::eLogLevel::Error, "Failed Create for TextureResource.");
			return false;
		}

		// アップロード用の中間バッファを取得
		const UINT numSubresources = static_cast<UINT>(metaData.mipLevels * metaData.arraySize);
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(mResource.Get(), 0, numSubresources);


		// データ転送の準備（全サブリソース分）
		std::vector<D3D12_SUBRESOURCE_DATA> subresources(numSubresources);
		for (UINT i = 0; i < numSubresources; ++i) {
			auto img = scratchImage.GetImage(i % metaData.mipLevels, i / metaData.mipLevels, 0);
			subresources[i].pData = img->pixels;
			subresources[i].RowPitch = static_cast<LONG_PTR>(img->rowPitch);
			subresources[i].SlicePitch = static_cast<LONG_PTR>(img->slicePitch);
		}

		if (dx12->UploadTextureData(mResource.Get(), subresources) == false) 
		{
			ECSE_LOG(System::eLogLevel::Error, "Failed to upload texture data to GPU.");
			return false;
		}

		// SRV作成
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = metaData.format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		// キューブマップかどうか
		if (metaData.IsCubemap()) {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MipLevels = static_cast<UINT>(metaData.mipLevels);
		}
		else {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = static_cast<UINT>(metaData.mipLevels);
		}

		// デスクリプタヒープから1枠確保してSRVを書き込む
		auto gdh = System::ServiceLocator::Get<GDescriptorHeapManager>();
		mHeapInfo = gdh->Issuance(1);
		device->CreateShaderResourceView(mResource.Get(), &srvDesc, gdh->GetCpuHandle(mHeapInfo));

		// サイズの取得
		mWidth = static_cast<float>(metaData.width);
		mHeight = static_cast<float>(metaData.height);

		ECSE_LOG(System::eLogLevel::Log, "Successfully loaded texture: " + FilePath.string());
		return true;

	}

	void Texture::Release()
	{
		mAllocation.Reset();
		mResource.Reset();
	}

	/// <summary>
	/// 割り当てられたインデックス
	/// </summary>
	/// <returns></returns>
	uint32_t Texture::GetDescriptorIndex() const
	{
		return mHeapInfo.Index;
	}

	/// <summary>
	/// 割り当てられたGpuハンドルの取得
	/// </summary>
	/// <returns></returns>
	D3D12_GPU_DESCRIPTOR_HANDLE Texture::GetGpuHandle()const
	{
		auto gdh = System::ServiceLocator::Get<GDescriptorHeapManager>();
		return gdh->GetGpuHandle(mHeapInfo);
	}

	/// テクスチャの幅
	/// </summary>
	/// <returns></returns>
	float Texture::GetWidth()const
	{
		return mWidth;
	}
	/// <summary>
	/// テクスチャの高さ
	/// </summary>
	/// <returns></returns>
	float Texture::GetHeight()const
	{
		return mHeight;
	}
}