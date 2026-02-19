#include "SpriteShader.hlsli"

float4 main(VStoPS input) : SV_Target
{
    float4 texColor = MainTexture.Sample(Sampler, input.UV);
    
    // インスタンスデータから届いた色を乗算
    float4 finalColor = texColor * input.Color;

    // ワンチャン最新のGPUだと逆に重くなるかも？最適化のせいで
    if (finalColor.a <= 0.0f)
    {
        discard;
    }
    
    // 届いた光度を適用
    finalColor.rgb *= input.Intensity;
    
    return finalColor;
}