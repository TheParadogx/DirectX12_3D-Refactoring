#include "SpriteShader.hlsli"

VStoPS main(VertexInput input, uint instID : SV_InstanceID)
{
    VStoPS output = (VStoPS) 0;

    // 自分のインスタンス用のデータを取得
    SpriteInstanceData data = gInstanceData[instID];

    // 変換
    output.Position = mul(float4(input.Position, 1.0f), data.WVP);
    output.UV = input.UV;
    
    // インスタンスごとの色と光度を渡す
    output.Color = data.Color;
    output.Intensity = data.Intensity;
    
    return output;
}