// 定数バッファ

// 基本的な変換行列 (b3)
// スロット 0 (b3): SceneConstant
cbuffer SceneBuffer : register(b3)
{
    matrix ViewProj; // C++ 側の SceneConstant.viewProjection
};

// スロット 1 (b4): MeshConstant
cbuffer MeshBuffer : register(b4)
{
    matrix World; // C++ 側の MeshConstant.world
};

// スロット 2 (b5): BoneConstant
cbuffer BoneBuffer : register(b5)
{
    matrix BoneTransforms[256];
};
// --- テクスチャリソース ( register t0, s0 ) ---
Texture2D DiffuseTexture : register(t0);
SamplerState LinearSampler : register(s0);

// --- 頂点レイアウト (SkeletalMeshVertexと一致させる) ---
struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
    float3 Tangent : TANGENT;
    uint4 Indices : BONE_INDICES; // インプットレイアウトの名前と一致
    float4 Weights : BONE_WEIGHTS; // インプットレイアウトの名前と一致
};

// 頂点シェーダーからピクセルシェーダーへの渡し
struct VS_OUTPUT
{
    float4 Position : SV_POSITION; // システム用頂点座標
    float2 UV : TEXCOORD0; // UV座標
    float3 Normal : NORMAL; // 法線 (ワールド空間)
    float3 WorldPos : POSITION; // ワールド座標
};