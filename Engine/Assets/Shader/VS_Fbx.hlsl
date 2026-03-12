#include "FbxShader.hlsli"

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    
    float4 localPos = float4(input.Position, 1.0f);
    float3 localNormal = input.Normal;

    float4 worldPos = mul(localPos, World);
    output.Position = mul(worldPos, ViewProj);
    
    output.Normal = normalize(mul(localNormal, (float3x3) World));
    
    output.UV = input.UV;
    output.Color = Color;
    
    return output;
}