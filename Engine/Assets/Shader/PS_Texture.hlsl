#include "SpriteShader.hlsli"

// ルートシグネチャのDescriptor Tableに紐づく
Texture2D MainTexture : register(t0);
// ルートシグネチャのStatic Samplerに紐づく
SamplerState Sampler : register(s0);

float4 main(VStoPS input) : SV_Target
{
    // テクスチャから色を取得
    float4 texColor = MainTexture.Sample(Sampler, input.UV);
    
    // 定数バッファの色を乗算
    float4 finalColor = texColor * input.Color;

    // アルファテスト（完全に透明なピクセルは描画処理を破棄）
    if (finalColor.a <= 0.0f)
    {
        discard;
    }
    
    // 光度をRGBに乗算
    finalColor.rgb *= Intensity;
    
    return finalColor;
}