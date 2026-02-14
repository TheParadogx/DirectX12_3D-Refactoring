#include "pch.h"
#include<System/Service/ServiceLocator.hpp>


/*
* typeid(T)が同一のモノとして扱われない可能性がホットリロードであるので
* その時はID方式に変更しようかな。
*/

namespace Ecse::System
{

	/// <summary>
	/// DLL境界を超えて唯一のマップ実体を保証するための内部関数。
	/// 将来的にホットリロードをやる可能性があるのでvoid*にしています
	/// </summary>
	static std::unordered_map<std::type_index, void*>* GetServiceMap()
	{
		// 最初の呼び出し時に一度だけnew
		// 関数内の static 変数は、その DLL 内のデータセグメントに一つだけ配置
		static std::unordered_map<std::type_index, void*>* sServices = new std::unordered_map<std::type_index, void*>();
		return sServices;
	}

	/// <summary>
	/// dll境界を超えつつ登録するためのヘルパー
	/// </summary>
	void ServiceLocator::RegisterInternal(std::type_index Type, void* Instance)
	{
		auto Services = GetServiceMap();

		//	中身がないやつが登録しようとしたら削除する
		if (Instance == nullptr)
		{
			Services->erase(Type);
			return;
		}

		// 演算子の優先度があるので(*sServices)
		(*Services)[Type] = Instance;
		// 	sServices->operator[](Type) = Instance;
	}

	/// <summary>
	/// dll境界を超えつつ取得するためのヘルパー
	/// </summary>
	void* ServiceLocator::GetInternal(std::type_index Type)
	{
		auto Services = GetServiceMap();

		//	検索
		auto it = Services->find(Type);
		if (it != Services->end())
		{
			return it->second;
		}

		//	検索失敗のログを出す

		return nullptr;
	}

}
