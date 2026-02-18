#pragma once
#include<Utility/Export/Export.hpp>

#include<DirectXMath.h>


/*
* ２Dと３Dは別のコンポーネントとして作ります。
*/

namespace Ecse::ECS
{
	/// <summary>
	/// 2D座標系用
	/// </summary>
	struct ENGINE_API Transform2D
	{
		// 座標
		DirectX::XMFLOAT3 Position = {0,0,0};
		// 全体倍率
		DirectX::XMFLOAT2 Scale = {1,1};
		// 回転
		float Rotation = 0.0f;

	};
}