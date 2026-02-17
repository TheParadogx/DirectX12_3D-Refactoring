#include "pch.h"
#include<Graphics/Shader/ShaderManager.hpp>

namespace Ecse::Graphics
{
	/// <summary>
    /// シェーダーの取得
    /// </summary>
    /// <param name="FileName">ファイルパス</param>
    /// <param name="EntryPoint">エントリー名</param>
    /// <param name="Target">バージョン</param>
    /// <returns></returns>
    Blob Ecse::Graphics::ShaderManager::GetShader(std::string_view FileName, std::string_view EntryPoint, std::string_view Target)
    {
		//	キー作成
		std::string key = CreateKey(FileName, EntryPoint, Target);

		// キャッシュ確認
		auto it = mShaders.find(key);
		if (it != mShaders.end())
		{
			return it->second;
		}

		//	無ければコンパイル
		Blob shaderBlob = CompileShader(FileName, EntryPoint, Target);

		mShaders.emplace(key, shaderBlob);
		return shaderBlob;
    }

	/// <summary>
	/// キーを作成する。
	/// ファイル名＋エントリポイント＋ターゲットでVSとかPSを区別する
	/// </summary>
	/// <param name="FileName">ファイルパス</param>
	/// <param name="EntryPoint">エントリー名</param>
	/// <param name="Target">バージョン</param>
	/// <returns></returns>
	std::string ShaderManager::CreateKey(std::string_view FileName, std::string_view EntryPoint, std::string_view Target)
	{
		return std::string(FileName) + "|" + std::string(EntryPoint) + "|" + std::string(Target);
	}

	/// <summary>
	/// シェーダーのコンパイル
	/// </summary>
	/// <param name="FileName">ファイルパス</param>
	/// <param name="EntryPoint">エントリー名</param>
	/// <param name="Target">バージョン</param>
	/// <returns></returns>
	Blob ShaderManager::CompileShader(std::string_view FileName, std::string_view EntryPoint, std::string_view Target)
	{
		wchar_t wideFileName[MAX_PATH];
		MultiByteToWideChar(CP_UTF8, 0, FileName.data(), static_cast<int>(FileName.size()), wideFileName, MAX_PATH);
		wideFileName[FileName.size()] = L'\0';

		Ecse::Blob shader = nullptr;
		Ecse::Blob error = nullptr;

		// コンパイル
		HRESULT hr = D3DCompileFromFile(
			wideFileName,
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			EntryPoint.data(), // entryPoint も string_view で受け取る
			Target.data(),      // target も string_view
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
			0,
			shader.GetAddressOf(),
			error.GetAddressOf()
		);

		if (FAILED(hr)) 
		{

			ECSE_LOG(Ecse::System::ELogLevel::Error, "Failed CreateShader.");
			return nullptr;
		}

		return shader;
	}
}

