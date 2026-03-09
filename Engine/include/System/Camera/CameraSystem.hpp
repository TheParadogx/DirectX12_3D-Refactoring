#pragma once
#include <entt/entt.hpp>
#include <DirectXMath.h>

namespace Ecse::System
{
	class CameraSystem
	{
    public:

        /// <summary>
        /// 全てのカメラエンティティの行列を更新する
        /// Viewは毎フレーム、Projectionは変更フラグがある時のみ計算
        /// </summary>
        static void Update(entt::registry& Registry);

        /// <summary>
        /// メインカメラに指定されているエンティティからViewとProjection行列を取得する
        /// </summary>
        /// <returns>有効なメインカメラが存在すればtrue</returns>
        static bool GetMainCameraMatrices(
            entt::registry& Registry,
            DirectX::XMMATRIX& OutView,
            DirectX::XMMATRIX& OutProj);
	};
}