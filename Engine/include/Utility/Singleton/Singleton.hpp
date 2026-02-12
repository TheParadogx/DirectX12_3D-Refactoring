#pragma once
#include<Utility/ServiceLocator/ServiceLocator.hpp>
#include<Utility/ServiceLocator/IService.hpp>

/*
* 派生クラスでもインスタンスを生成できないようにする処理のマクロ定義
* これを継承先privateで呼び出す。
*/
#define GENERATE_SINGLETON_BODY(Class)\
private:\
friend ::Ecse::Utility::Singleton<Class>;\
Class()noexcept = default;\
virtual ~Class()noexcept  = default;\
Class(const Class&) = delete;\
Class(Class&&) = delete;\
Class& operator=(const Class&) = delete;\
Class& operator=(Class&&) = delete;

namespace Ecse::Utility
{
	/// <summary>
	/// Singletonテンプレート
	/// </summary>
	/// <typeparam name="T">継承先のクラス</typeparam>
	template<typename T>
	class Singleton : public ISystem
	{
	public:
		/// <summary>
		/// 生成と同時にサービスロケータへ登録する
		/// </summary>
		/// <returns></returns>
		static bool Create()
		{
			if (ServiceLocator::Get<T>() == nullptr)
			{
				T* instance = new T();
				ServiceLocator::Register<T>(instance);
				instance->OnCreate();
				return true;
			}
			return false;
		}

		/// <summary>
		/// 登録解除と同時にメモリ解放
		/// </summary>
		static void Release()
		{
			T* instance = ServiceLocator::Get<T>();
			if (instance != nullptr)
			{
				instance->OnDestroy();
				ServiceLocator::Unregister<T>();
				delete instance;
			}
		}

		/// <summary>
		/// ServiceLocatorからインスタンスを取得
		/// </summary>
		/// <returns></returns>
		static T* Get()
		{
			return ServiceLocator::Get<T>();
		}

	protected:
		Singleton() noexcept = default;
		virtual ~Singleton() noexcept = default;

		/// <summary>
		/// 初期化（実質コンストラクタ）
		/// </summary>
		virtual void OnCreate() {}

		/// <summary>
		/// 終了処理（実質デストラクタ）
		/// </summary>
		virtual void OnDestroy() {}

	private:
		// コピーとムーブの禁止
		Singleton(const Singleton&) = delete;
		Singleton(Singleton&&) = delete;
		Singleton& operator=(const Singleton&) = delete;
		Singleton& operator=(Singleton&&) = delete;
	};
}
