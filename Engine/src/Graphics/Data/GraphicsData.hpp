#pragma once
#include <DirectXMath.h>
#include<Graphics/Color/Color.hpp>

namespace Ecse::Graphics
{

	/// <summary>
	/// 2Dスプライト描画用の頂点構造体
	/// </summary>
	struct SpriteVertex
	{
        DirectX::XMFLOAT3 Pos;   // 座標
        DirectX::XMFLOAT2 UV;    // テクスチャ座標
        Color Color; // 頂点カラー
	};

    struct SpriteShaderData {
       DirectX::XMMATRIX WVP;    // ワールド・ビュー・プロジェクション 64
       Color Color;  // 色 16
       float Intensity; // 光度 4
       float Padding[3];           // 12byte
    };
}