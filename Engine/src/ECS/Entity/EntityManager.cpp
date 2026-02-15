#include "pch.h"
#include<ECS/Entity/EntityManager.hpp>
#include<ECS/Tag/SceneTags.hpp>

namespace Ecse::ECS
{

	/// <summary>
	/// 初期化
	/// </summary>
	/// <returns></returns>
	bool EntityManager::Initialize()
	{
		mRegistry.clear();
		mDestroyQueue.clear();
		return true;
	}

	/// <summary>
	/// 通常のエンティティ作成
	/// </summary>
	/// <returns></returns>
	entt::entity EntityManager::CreateEntity()
	{
		return mRegistry.create();
	}

	/// <summary>
	/// 永続（シーンをまたぐ）の作成
	/// </summary>
	/// <returns></returns>
	entt::entity EntityManager::CreatePersistentEntity()
	{
		auto entity = mRegistry.create();
		mRegistry.emplace<PersistentTag>(entity);
		return entity;
	}

	/// <summary>
	/// エンティティの遅延削除
	/// </summary>
	/// <param name="entity"></param>
	void EntityManager::DestroyDeferred(entt::entity entity)
	{
		if (mRegistry.valid(entity))
		{
			mDestroyQueue.push_back(entity);
		}
	}

	/// <summary>
	/// 状態更新（削除など）
	/// </summary>
	void EntityManager::Update()
	{
		//	空なら何もしない
		if (mDestroyQueue.empty()) return;

		for (auto entity : mDestroyQueue)
		{
			//	有効なエンティティなら
			if (mRegistry.valid(entity))
			{
				//	削除
				mRegistry.destroy(entity);
			}
		}
		mDestroyQueue.clear();
	}

	/// <summary>
	/// Localエンティティの削除
	/// </summary>
	void EntityManager::ClearLocalEntities()
	{
		// PersistentTag を持っていないエンティティをすべて抽出
		auto view = mRegistry.view<entt::entity>(entt::exclude<PersistentTag>);

		// イテレータを渡して一括削除
		mRegistry.destroy(view.begin(), view.end());
	}

	/// <summary>
	/// 全ての削除
	/// </summary>
	void EntityManager::AllClear()
	{
		mRegistry.clear();
		mDestroyQueue.clear();
	}

	/// <summary>
	/// Registryへのアクセス
	/// </summary>
	entt::registry& EntityManager::GetRegistry()
	{
		return mRegistry;
	}

}