#pragma once
#include <DirectXMath.h>
#include<Graphics/Color/Color.hpp>

namespace EcseGraphics
{
    struct SpriteShaderData {
        DirectX::XMMATRIX WVP;    // ワールド・ビュー・プロジェクション
        DirectX::XMFLOAT4 Color;  // 色
    };
}