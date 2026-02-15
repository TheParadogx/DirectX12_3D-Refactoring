#pragma once
#include<System/Service/ServiceProvider.hpp>
#include<memory>
#include<unordered_map>
#include<filesystem>
#include<concepts>
#include<mutex>
#include<System/Log/Logger.hpp>
#include"IResource.hpp"

namespace Ecse::System
{
	/// <summary>
	/// リソース以外を入れられないようにするためのコンセプト
	/// </summary>
	template<typename T>
	concept IsResource = std::derived_from<T, IResource>;

	/// <summary>
	/// 各リソースを管理するクラス
	/// </summary>
	class ResourceManager : public ServiceProvider<ResourceManager>
	{
	public:
		/// <summary>
		/// 型を指定して読み込む
		/// </summary>
		/// <typeparam name="T">読み込みたい型</typeparam>
		/// <param name="FilePath"></param>
		/// <returns></returns>
		template<IsResource T>
		static T* Load(const std::filesystem::path& FilePath)
		{

			if (std::filesystem::exists(FilePath) == false) 
			{
				ECSE_LOG(ELogLevel::Error, "File not found: {}", FilePath.string());
				return nullptr;
			}

			// 相対パス取得
			std::string key = std::filesystem::absolute(FilePath).generic_string();
			std::lock_guard<std::mutex> lock(sMutex);
			auto& cache = GetCache<T>();

			//	検索
			auto it = cache.find(key);
			if (it != cache.end()) 
			{
				// あれば返す
				return it->second.get();
			}

			// 新規ロード
			auto res = std::make_unique<T>();
			if (res->Create(FilePath)) 
			{
				T* ptr = res.get();
				cache[key] = std::move(res);
				ECSE_LOG(ELogLevel::Info, "Resource Loaded: {}", key);
				return ptr;
			}

			// 5. ロード失敗時のログ
			ECSE_LOG(ELogLevel::Error, "Failed to create resource from: {}", key);
			return nullptr;
		}



	private:
		/// <summary>
		/// T型の読み込んだリソースのマップ
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <returns></returns>
		template<IsResource T>
		static  std::unordered_map<std::string, std::unique_ptr<T>>& GetCache()
		{
			static std::unordered_map<std::string, std::unique_ptr<T>> instance;
			return instance;
		}


		static std::mutex sMutex;
	};

}