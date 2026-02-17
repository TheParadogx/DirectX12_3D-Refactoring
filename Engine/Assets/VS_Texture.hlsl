#include "SpriteShader.hlsli"

VStoPS main(VertexInput input)
{
    VStoPS output = (VStoPS) 0;

    // 1x1の板頂点を、CPUで計算した行列(SRT * Proj)で変換
    output.Position = mul(float4(input.Position, 1.0f), WVP);

    // UVはそのまま渡す
    output.UV = input.UV;
    
    // 色情報をピクセルシェーダーへ渡す
    output.Color = MaterialColor;

    return output;
}