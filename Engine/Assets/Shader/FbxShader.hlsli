// --- 定数バッファ定義 ---

// Slot 0 (b3): シーン共通（カメラ等）
cbuffer SceneBuffer : register(b3)
{
    matrix ViewProj;
};

// Slot 1 (b4): メッシュ固有
cbuffer MeshBuffer : register(b4)
{
    matrix World;
    float4 Color;
    int IsSkinned;
    float3 MeshPadding; // 16バイトアライメント用
};

// Slot 2 (b5): ボーン行列
cbuffer BoneBuffer : register(b5)
{
    matrix Bones[256];
};

// --- 構造体定義 ---

struct VS_INPUT
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    uint4 BoneIndex : BONE_INDEX;
    float4 Weight : WEIGHT;
};

struct VS_OUTPUT
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
};