#pragma once
#include<Utility/Export/Export.hpp>
#include<typeindex>

#include"IService.hpp"
#include<concepts>

namespace Ecse::Utility
{
	/// <summary>
	/// どの型でも渡せたらやばいので特定の基底クラスに縛ります。
	/// </summary>
	template<typename T>
	concept IsEngineSystem = std::derived_from<T, IService>;

	/// <summary>
	/// 各システムのインスタンスを一括管理をするクラス
	/// DLL化のことを考えて境界を超えれるようにしてます。
	/// </summary>
	class ENGINE_API ServiceLocator
	{
	public:
		/// <summary>
		/// 型情報を保持した状態でvoid*に変換して格納
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="Instance"></param>
		template<IsEngineSystem  T>
		static void Register(T* Instance)
		{
			RegisterInternal(typeid(T));
		}

		/// <summary>
		/// 登録されているインスタンスを検索してあればポインタを返す
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <returns></returns>
		template<IsEngineSystem  T>
		static T* Get()
		{
			return static_cast<T*>(GetInternal((typeid(T))));
		}

		/// <summary>
		/// 登録を解除する
		/// </summary>
		/// <typeparam name="T"></typeparam>
		template<IsEngineSystem  T>
		static void Unregister() 
		{
			RegisterInternal((typeid(T)), nullptr);
		}

	private:
		/// <summary>
		/// dll境界を超えつつ登録するためのヘルパー
		/// </summary>
		static void RegisterInternal(std::type_index Type, void* Instance);

		/// <summary>
		/// dll境界を超えつつ取得するためのヘルパー
		/// </summary>
		static void* GetInternal(std::type_index Type);
	};
}