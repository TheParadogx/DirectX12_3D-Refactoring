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
	};

    struct SpriteShaderData {
        DirectX::XMFLOAT4X4 WVP;    // ワールド・ビュー・プロジェクション 64
       Color Color;  // 色 16
       float Intensity; // 光度 4
       float Padding[3];           // 12byte
    };

	/// <summary>
	/// FBXの頂点構造体
	/// </summary>
#pragma pack(push, 1)

    struct SkeletalMeshVertex {
        DirectX::XMFLOAT3 Position; // 12byte
        DirectX::XMFLOAT3 Normal;   // 12byte
        DirectX::XMFLOAT2 UV;       // 8byte
        DirectX::XMFLOAT3 Tangent;  // 12byte
        uint32_t BoneIndices[4];    // 16byte
        float BoneWeights[4];       // 16byte
    };

#pragma pack(pop)

}