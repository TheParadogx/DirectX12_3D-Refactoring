#include "pch.h"
#include "Utility/ServiceLocator/ServiceLocator.hpp"


namespace Ecse::Utility
{
	//	生ポインタなのはホットリロードを実装するときに
	//  アンロードでもインスタンスを破棄されずに同じ実体をさしたいので
	static std::unordered_map<std::type_index, void*>* sServices = nullptr;


	/// <summary>
	/// dll境界を超えつつ登録するためのヘルパー
	/// </summary>
	void ServiceLocator::RegisterInternal(std::type_index Type, void* Instance)
	{
		if (sServices == nullptr)
		{
			sServices = new std::unordered_map<std::type_index, void*>();
		}

		//	中身がないやつが登録しようとしたら削除する
		if (Instance == nullptr)
		{
			sServices->erase(Type);
			return;
		}

		// 演算子の優先度があるので(*sServices)
		(*sServices)[Type] = Instance;
		// 	sServices->operator[](Type) = Instance;
	}

	/// <summary>
	/// dll境界を超えつつ取得するためのヘルパー
	/// </summary>
	void* ServiceLocator::GetInternal(std::type_index Type)
	{
		if (sServices == nullptr) return nullptr;

		//	検索
		auto it = sServices->find(Type);
		if (it != sServices->end())
		{
			return it->second;
		}

		//	検索失敗のログを出す

		return nullptr;
	}

}
