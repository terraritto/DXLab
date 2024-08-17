// /T vs_5_0 
#include "../CommonDX12.hlsli"

ConstantBuffer<ShadowParameter> shadowParam : register(b0);

VSShadowOutput main(VSInput In)
{
    VSShadowOutput result;
    result.Position = mul(shadowParam.world, In.Position);
    result.Position = mul(shadowParam.view, result.Position);
    result.Position = mul(shadowParam.proj, result.Position);
    
    result.UV = In.UV;
    result.Normal = mul(shadowParam.world, In.Normal);
    
    return result;
}
