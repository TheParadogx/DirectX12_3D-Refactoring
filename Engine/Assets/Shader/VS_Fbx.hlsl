#include"FbxShader.hlsli"

PSInput VSMain(VSInput input)
{
    PSInput output;

    // 位置のスキニング
    matrix boneMatrix =
        BoneTransforms[input.BoneIndices.x] * input.BoneWeights.x +
        BoneTransforms[input.BoneIndices.y] * input.BoneWeights.y +
        BoneTransforms[input.BoneIndices.z] * input.BoneWeights.z +
        BoneTransforms[input.BoneIndices.w] * input.BoneWeights.w;

    float4 localPos = float4(input.Position, 1.0f);
    float4 skinnedPos = mul(localPos, boneMatrix);
    float4 worldPos = mul(skinnedPos, World);
    output.Position = mul(worldPos, ViewProjection);

    // 法線のスキニング (各ボーン変形後の合成方式)
    // 各ボーンの回転・スケール成分のみを法線に適用し、重みでブレンドする
    float3 skinnedNormal =
        input.BoneWeights.x * mul((float3x3) BoneTransforms[input.BoneIndices.x], input.Normal) +
        input.BoneWeights.y * mul((float3x3) BoneTransforms[input.BoneIndices.y], input.Normal) +
        input.BoneWeights.z * mul((float3x3) BoneTransforms[input.BoneIndices.z], input.Normal) +
        input.BoneWeights.w * mul((float3x3) BoneTransforms[input.BoneIndices.w], input.Normal);

    // ワールド空間への変換と正規化
    // 頂点シェーダーで正規化しておくことで、ピクセルシェーダーの負荷を軽減
    output.Normal = normalize(mul((float3x3) World, skinnedNormal));

    output.UV = input.UV;

    return output;
}