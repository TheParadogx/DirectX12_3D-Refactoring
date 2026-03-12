#include "FbxShader.hlsli"

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output;

    // --- スキニング計算 ---
    float weightSum = input.Weights.x + input.Weights.y + input.Weights.z + input.Weights.w;
    float4 w = input.Weights / (weightSum > 0.0f ? weightSum : 1.0f);

    float4x4 skinMatrix =
        BoneTransforms[input.Indices.x] * w.x +
        BoneTransforms[input.Indices.y] * w.y +
        BoneTransforms[input.Indices.z] * w.z +
        BoneTransforms[input.Indices.w] * w.w;

    // --- 座標変換 ---
    float4 pos = mul(float4(input.Position, 1.0f), skinMatrix);
    pos = mul(pos, World);
    output.WorldPos = pos.xyz;
    output.Position = mul(pos, ViewProj);

    // --- 法線変換 (逆行列なし) ---
    // スケールが uniform の場合はこれで OK
    float3 normal = mul(input.Normal, (float3x3) skinMatrix);
    normal = mul(normal, (float3x3) World);
    output.Normal = normalize(normal);

    output.UV = input.UV;

    return output;
}