// 定数バッファ

cbuffer SceneConstant : register(b3)
{
    matrix ViewProjection;
};

cbuffer MeshConstant : register(b4)
{
    matrix World;
};

cbuffer BoneConstant : register(b5)
{
    matrix BoneTransforms[128];
};

// 入出力構造体
struct VSInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
    uint4 BoneIndices : BONE_INDICES;
    float4 BoneWeights : BONE_WEIGHTS;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL; // VSで正規化済み
    float2 UV : TEXCOORD;
};