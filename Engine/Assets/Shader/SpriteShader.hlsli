
// 頂点シェーダーへの入力
struct VertexInput
{
    float3 Position : POSITION; // 12byte
    float2 UV : TEXCOORD; // 8byte
};

// 頂点からピクセルへの出力
struct VStoPS
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
    float4 Color : COLOR;
    float Intensity : INTENSITY; // PSで使うので渡す
};

// インスタンスデータ
struct SpriteInstanceData
{
    float4x4 WVP;
    float4 Color;
    float Intensity;
    float3 Padding; // 16byte境界用
};

// 
StructuredBuffer<SpriteInstanceData> gInstanceData : register(t1);

// register(t0) : DescriptorTable
Texture2D MainTexture : register(t0);
SamplerState Sampler : register(s0);