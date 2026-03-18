#pragma once
#include<System/Service/ServiceProvider.hpp>
#include<memory>
#include<unordered_map>
#include<filesystem>
#include<concepts>
#include<mutex>
#include<System/Log/Logger.hpp>
#include"IResource.hpp"
#include<Utility/Export/Export.hpp>


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
	class ENGINE_API ResourceManager : public ServiceProvider<ResourceManager>
	{
	public:
		/// <summary>
		/// 型を指定して読み込む
		/// </summary>
		/// <typeparam name="T">読み込みたい型</typeparam>
		/// <param name="FilePath"></param>
		/// <returns></returns>
		template<IsResource T>
		static T* Load(const std::filesystem::path& path)
		{
			auto key = std::filesystem::weakly_canonical(path).string();

			auto& cache = GetCache<T>();
			auto& mutex = GetMutex<T>();

			std::lock_guard lock(mutex);

			if (auto it = cache.find(key); it != cache.end())
				return it->second.get();

			auto res = std::make_unique<T>();

			if (!res->Create(path))
				return nullptr;

			auto ptr = res.get();
			cache.emplace(key, std::move(res));

			return ptr;
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