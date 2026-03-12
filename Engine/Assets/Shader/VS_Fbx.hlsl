#include "FbxShader.hlsli"

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    
    float4 skinnedPos = float4(0, 0, 0, 0);
    float3 skinnedNormal = float3(0, 0, 0);

    if (IsSkinned)
    {
        // 4つのボーンの影響を合計する
        for (int i = 0; i < 4; i++)
        {
            // 影響度が 0 より大きい場合のみ計算（最適化のため）
            if (input.Weight[i] > 0.0f)
            {
                // 頂点座標の計算
                skinnedPos += mul(float4(input.Position, 1.0f), Bones[input.BoneIndex[i]]) * input.Weight[i];
                
                // 法線の計算（平行移動成分を無視するため 3x3 にキャスト）
                skinnedNormal += mul(input.Normal, (float3x3) Bones[input.BoneIndex[i]]) * input.Weight[i];
            }
        }
    }
    else
    {
        skinnedPos = float4(input.Position, 1.0f);
        skinnedNormal = input.Normal;
    }

    // 最終的なワールド・スクリーン座標変換
    float4 worldPos = mul(skinnedPos, World);
    output.Position = mul(worldPos, ViewProj);
    
    // 法線の正規化
    output.Normal = normalize(mul(skinnedNormal, (float3x3) World));
    
    output.UV = input.UV;
    output.Color = Color;
    
    return output;
}