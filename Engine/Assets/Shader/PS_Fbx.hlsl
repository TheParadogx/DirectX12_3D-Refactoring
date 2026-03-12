#include"FbxShader.hlsli"
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

float4 PS_main(VS_OUTPUT input) : SV_TARGET
{
    // --- テクスチャサンプリング ---
    float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.UV);
    
    // --- 簡易的なライティング (Diffuseのみ) ---
    float3 lightDir = normalize(float3(1.0f, 1.0f, -1.0f)); // 簡易的な平行光源
    float3 normal = normalize(input.Normal);
    float NdotL = saturate(dot(normal, lightDir)); // 法線とライト方向の内積
    
    // アンビエント光 (環境光)
    float3 ambient = float3(0.2f, 0.2f, 0.2f);
    
    // 最終的な色の計算
    float3 finalColor = diffuseColor.xyz * (NdotL + ambient);
    
    //return float4(1, 1, 1, 1);
    return float4(finalColor, diffuseColor.a); // アルファ値はテクスチャのまま
}