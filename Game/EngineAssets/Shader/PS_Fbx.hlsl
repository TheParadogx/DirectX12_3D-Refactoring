#include"FbxShader.hlsli"
// --- リソース ---
Texture2D DiffuseTexture : register(t0);
SamplerState LinearSampler : register(s0);

float4 PS_main(VS_OUTPUT input) : SV_TARGET {
    // 1. テクスチャサンプリング
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.UV);
    
    // 2. 簡易ライティング (とりあえず斜め上からの平行光)
    float3 lightDir = normalize(float3(0.5f, -1.0f, 0.5f));
    float diffuse = saturate(dot(input.Normal, -lightDir));
    
    // アンビエント（環境光）を少し足して真っ黒を防ぐ
    float ambient = 0.3f;
    float3 finalColor = texColor.rgb * (diffuse + ambient);

    return float4(finalColor, texColor.a);
}