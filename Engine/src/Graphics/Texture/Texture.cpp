#include "pch.h"
#include<Graphics/Texture/Texture.hpp>
#include<System/Service/ServiceLocator.hpp>
#include<Graphics/DX12/DX12.hpp>

namespace fs = ::std::filesystem;
namespace DirectXTex = ::DirectX;

namespace Ecse::Graphics
{
	/// <summary>
	/// TypelessフォーマットをSRV用に変換
	/// </summary>
	/// <param name="format">今のフォーマット</param>
	/// <returns></returns>
	DXGI_FORMAT GetSRVFormat(DXGI_FORMAT format) 
	{
		switch (format) {
		case DXGI_FORMAT_R8G8B8A8_TYPELESS: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case DXGI_FORMAT_BC1_TYPELESS:     return DXGI_FORMAT_BC1_UNORM;
		case DXGI_FORMAT_BC2_TYPELESS:     return DXGI_FORMAT_BC2_UNORM;
		case DXGI_FORMAT_BC3_TYPELESS:     return DXGI_FORMAT_BC3_UNORM;
		case DXGI_FORMAT_BC7_TYPELESS:     return DXGI_FORMAT_BC7_UNORM;
			// 必要に応じて sRGB への変換ロジックをここに追加
		default: return format;
		}
	}

	Texture::Texture()
		:mHeapInfo()
		, mReource(nullptr)
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
		//	フォルダの存在判定
		if (fs::exists(FilePath) == false)
		{
			ECSE_LOG(System::ELogLevel::Error, "NotExists Texture. : " + FilePath.string());
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
			ECSE_LOG(System::ELogLevel::Error, "Fialed LoadTexture.");
			return false;
		}

		const DirectXTex::Image* Image = scratchImage.GetImage(0, 0, 0);
		Graphics::DX12* dx12 = System::ServiceLocator::Get<Graphics::DX12>();
		ID3D12Device* device = dx12->GetDevice();

		// GPUリソースの作成

		
	


		return true;
	}
}