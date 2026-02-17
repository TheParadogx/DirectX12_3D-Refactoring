#pragma once
#include <DirectXMath.h>
#include<Graphics/Color/Color.hpp>

namespace Ecse::Graphics
{
    struct SpriteShaderData {
        DirectX::XMMATRIX WVP;    // ワールド・ビュー・プロジェクション 64
       Color Color;  // 色 16
       float Intensity; // 光度 4
       float Padding[3];           // 12byte
    };
}