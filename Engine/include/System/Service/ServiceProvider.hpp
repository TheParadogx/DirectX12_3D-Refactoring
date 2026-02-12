#pragma once

#include"IService.hpp"
#include"ServiceLocator.hpp"
#include<memory>

/*
* 派生クラスでもインスタンスを生成できないようにする処理のマクロ定義
* これを継承先privateで呼び出す。
*/

namespace Ecse::System
{
	/// <summary>
	/// サービスの登録をするクラス。
	/// このクラスからのアクセスは不可。
	/// アクセスはすべてServiceLocatorから行います。
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template<typename T>
	class ServiceProvider : public IService
	{
	public:
		ServiceProvider() noexcept = default;
		virtual ~ServiceProvider() noexcept = default;

		/// <summary>
		/// 生成と同時にサービスロケータへ登録する
		/// </summary>
		/// <returns></returns>
		static bool Create()
		{
			//	既に生成済み
			if (sInstance != nullptr) return false;

			sInstance = std::make_unique<T>();
			sInstance->OnCreate();
			ServiceLocator::Register<T>(sInstance.get());
			return true;
		}

		/// <summary>
		/// 登録解除と同時にメモリ解放
		/// </summary>
		static void Release()
		{
			if (sInstance == nullptr) return;
			ServiceLocator::Unregister<T>();
			sInstance->OnDestroy();
			sInstance.reset();
		}

		/// <summary>
		/// 生成されているかどうかの判定
		/// </summary>
		/// <returns></returns>
		static bool IsCreated() noexcept
		{
			return sInstance != nullptr;
		}

	protected:
		/// <summary>
		/// 初期化（実質コンストラクタ）
		/// </summary>
		virtual void OnCreate() {}

		/// <summary>
		/// 終了処理（実質デストラクタ）
		/// </summary>
		virtual void OnDestroy() {}

	private:
		static std::unique_ptr<T> sInstance;

	};

	template<typename T>
	std::unique_ptr<T> ServiceProvider<T>::sInstance = nullptr;

}
