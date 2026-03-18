#pragma once
#include<entt/entt.hpp>

namespace Ecse::System
{
	class FbxAnimationSystem
	{
	public:
		/// <summary>
		/// アニメーション更新と階層スキニング行列の計算
		/// </summary>
		/// <param name="registry"></param>
		/// <param name="deltaTime"></param>
		static void Update(entt::registry& registry, float deltaTime);
	};
}

