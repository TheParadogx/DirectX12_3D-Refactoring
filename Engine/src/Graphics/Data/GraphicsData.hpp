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
	struct SkeletalMeshVertex {
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 UV;
		DirectX::XMFLOAT3 Tangent;
		int32_t  BoneIndices[4];
		float    BoneWeights[4];
	};

}