#include "../CommonDX12.hlsli"

ConstantBuffer<ShadowParameter> sceneParam : register(b0);

float4 main(VSShadowOutput In) : SV_TARGET
{
    float posz = In.Position.z / In.Position.w;
    return float4(posz, posz, posz, 1.0f);
}