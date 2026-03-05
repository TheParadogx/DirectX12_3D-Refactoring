#pragma once

#include<Utility/Export/Export.hpp>
#include<System/Service/ServiceProvider.hpp>
#include<unordered_map>
#include<string>
#include<memory>
#include<mutex>
#include<filesystem>

namespace Ecse::Graphics
{
	class Texture;

	struct TextureEntry
	{
		std::unique_ptr<Texture> texture;      // 実データへの所有権
		std::filesystem::path path;            // ホットリロード用の絶対パス
		std::filesystem::file_time_type lastWriteTime; // 成功時のファイル更新時間
	};

	/// <summary>
	/// テクスチャ管理
	/// </summary>
	class ENGINE_API TextureManager : public System::ServiceProvider<TextureManager>
	{
		ECSE_SERVICE_ACCESS(TextureManager);
	public:

		/// <summary>
		/// テクスチャの取得、ミロードならロードする。
		/// </summary>
		/// <param name="FilePath">ファイルパス</param>
		/// <returns>参照用のポインタ</returns>
		Texture* GetOrLoad(const std::filesystem::path& FilePath);

		/// <summary>
		/// ホットリロード
		/// </summary>
		void HotReload();

		/// <summary>
		/// キャッシュの削除
		/// </summary>
		void Clear();
	private:
		/// <summary>
		/// リソース全体
		/// </summary>
		std::unordered_map<std::string, TextureEntry> mResources;
		/// <summary>
		/// 排他制御
		/// </summary>
		std::mutex mMutex;
	};
}