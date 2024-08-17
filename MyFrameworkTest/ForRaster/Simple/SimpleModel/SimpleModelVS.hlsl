// /T vs_5_0 
#include "../../../../Shader/CommonDX12.hlsli"

ConstantBuffer<ShaderParameter> sceneParam : register(b0);

VSOutput main(VSInput In)
{
	VSOutput result = (VSOutput)0;
    float4 position = In.Position;
    position = mul(sceneParam.world, position);
    result.WorldPos = position;

    position = mul(sceneParam.view, position);
    position = mul(sceneParam.proj, position);
	result.Position = position;
	
	float3 N = normalize(mul((float3x3)sceneParam.world, In.Normal.xyz));
	float3 T = normalize(mul((float3x3)sceneParam.world, In.Tangent.xyz));
	float3 B = normalize(cross(N, T));
	//result.Normal = float4(normalize(mul((float3x3)sceneParam.world, In.Normal.xyz)), 1.0f);
	result.UV = In.UV;
	result.TangentBasis = float3x3(T, B, N);	

	return result;
}