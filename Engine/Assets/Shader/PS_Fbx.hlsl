#include"FbxShader.hlsli"
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

float4 PSMain(PSInput input) : SV_TARGET
{
    // ディレクショナルライト設定 (本来は定数バッファから持ってくるべき値。今は固定ライトにします)
    float3 lightDir = normalize(float3(1.0f, -1.0f, 1.0f));
    
    // 法線は線形補間により長さが1未満になる可能性があるため、ここでも一応再正規化
    float3 N = normalize(input.Normal);
    float3 L = normalize(-lightDir);
    
    float diffuse = max(dot(N, L), 0.3f); // 0.3はAmbient成分

    float4 texColor = gTexture.Sample(gSampler, input.UV);
    
    return float4(1,1,1,1);
    //return float4(texColor.rgb * diffuse, texColor.a);
}