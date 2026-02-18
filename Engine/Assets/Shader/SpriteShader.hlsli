
// 頂点シェーダーへの入力
struct VertexInput
{
    float3 Position : POSITION; // 12byte
    float2 UV : TEXCOORD; // 8byte
    float4 Color : COLOR;
};

// 頂点からピクセルへの出力
struct VStoPS
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
    float4 Color : COLOR;
};

// 定数バッファ (b0)
cbuffer SpriteData : register(b0)
{
    float4x4 WVP; // 64byte
    float4 MaterialColor; // 16byte (RGBA)
    float Intensity; // 4byte
    float3 Padding; // 12byte (16byte境界へのパディング)
};