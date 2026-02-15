#pragma once

#include<entt/entt.hpp>
#include<System/Service/ServiceProvider.hpp>
#include<vector>

namespace Ecse::ECS
{
	/// <summary>
	/// レジスターの管理と生成破棄
	/// </summary>
	class EntityManager : public System::ServiceProvider<EntityManager>
	{
		ECSE_SERVICE_ACCESS(EntityManager);
	public:

		/// <summary>
		/// 初期化
		/// </summary>
		/// <returns></returns>
		bool Initialize();

		/// <summary>
		/// 通常のエンティティ作成
		/// </summary>
		/// <returns></returns>
		[[nodiscard]] entt::entity CreateEntity();

		/// <summary>
		/// 永続（シーンをまたぐ）の作成
		/// </summary>
		/// <returns></returns>
		[[nodiscard]] entt::entity CreatePersistentEntity();

		/// <summary>
		/// エンティティの遅延削除
		/// </summary>
		/// <param name="entity"></param>
		void DestroyDeferred(entt::entity entity);

		/// <summary>
		/// 状態更新（削除など）
		/// </summary>
		void Update();

		/// <summary>
		/// Localエンティティの削除
		/// </summary>
		void ClearLocalEntities();

		/// <summary>
		/// 全ての削除
		/// </summary>
		void AllClear();

		/// <summary>
		/// Registryへのアクセス
		/// </summary>
		[[nodiscard]] entt::registry& GetRegistry();
	private:
		/// <summary>
		/// 全ての管理のレジストリ
		/// </summary>
		entt::registry mRegistry;
		/// <summary>
		/// 削除予定のエンティティリスト
		/// </summary>
		std::vector<entt::entity> mDestroyQueue;
	};
}


