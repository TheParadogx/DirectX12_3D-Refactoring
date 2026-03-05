#include "pch.h"
#include<Graphics/Texture/Manager/TextureManager.hpp>
#include<Graphics/Texture/Texture.hpp>

namespace Ecse::Graphics
{
	/// <summary>
	/// テクスチャの取得、ミロードならロードする。
	/// </summary>
	/// <param name="FilePath">ファイルパス</param>
	/// <returns>参照用のポインタ</returns>
    Texture* TextureManager::GetOrLoad(const std::filesystem::path& FilePath)
    {
		std::string key = std::filesystem::absolute(FilePath).generic_string();

		// 検索
		{
			std::lock_guard<std::mutex> lock(mMutex);
			auto it = mResources.find(key);
			if (it != mResources.end())
			{
				return it->second.texture.get();
			}
		}

		// ロード
		auto newTexture = std::make_unique<Texture>();
		if (!newTexture->Create(FilePath))
		{
			return nullptr;
		}

		// 初期ロード成功後に時間を取得
		auto writeTime = std::filesystem::last_write_time(FilePath);

		// 登録
		{
			std::lock_guard<std::mutex> lock(mMutex);
			auto [it, inserted] = mResources.try_emplace(key, TextureEntry
				{
					std::move(newTexture),
					FilePath,
					writeTime
				});

			return it->second.texture.get();
		}

		return nullptr;
    }

	/// <summary>
	/// ホットリロード
	/// </summary>
	void TextureManager::HotReload()
	{
		std::vector<std::string> reloadKeys;

		// 更新チェック
		{
			std::lock_guard<std::mutex> lock(mMutex);
			for (auto& [key, entry] : mResources)
			{
				try
				{
					auto currentWriteTime = std::filesystem::last_write_time(entry.path);
					if (currentWriteTime != entry.lastWriteTime)
					{
						// 成功するかわからないので、ここではキーのみ控える
						reloadKeys.push_back(key);
					}
				}
				catch (const std::filesystem::filesystem_error&) { continue; }
			}
		}

		// リロード実行
		for (const auto& key : reloadKeys)
		{
			std::filesystem::path targetPath;
			{
				std::lock_guard<std::mutex> lock(mMutex);
				if (auto it = mResources.find(key); it != mResources.end()) {
					targetPath = it->second.path;
				}
				else { continue; }
			}

			auto newTexture = std::make_unique<Texture>();

			// 成功した場合のみ
			if (newTexture->Create(targetPath))
			{
				std::lock_guard<std::mutex> lock(mMutex);
				if (auto it = mResources.find(key); it != mResources.end())
				{
					// 成功を確定させる（アトミックな更新）
					it->second.texture = std::move(newTexture);
					it->second.lastWriteTime = std::filesystem::last_write_time(it->second.path);
				}
			}
		}
	}

	/// <summary>
	/// キャッシュの削除
	/// </summary>
	void TextureManager::Clear()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mResources.clear();
	}

}
