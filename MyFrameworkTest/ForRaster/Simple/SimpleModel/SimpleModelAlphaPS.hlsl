#include "../../../../Shader/BRDFDX12.hlsli"
#include "../../../../Shader/CommonDX12.hlsli"
#include "../../../../Shader/Constant.hlsli"
#include "../../../../Shader/Light.hlsli"
#include "../../../../Shader/Shadow/ShadowCommon.hlsli"

Texture2D ColorTexture : register(t0);
Texture2D DiffuseTexture : register(t1);
Texture2D NormalTexture : register(t2);
Texture2D ShadowTexture : register(t3);
SamplerState samp : register(s0);
ConstantBuffer<ShaderParameter> sceneParam : register(b0);
ConstantBuffer<SceneShadowParameter> shadowParam : register(b1);
ConstantBuffer<MaterialParameter> materialParam : register(b2);
ConstantBuffer<LightData> lightBuffer : register(b3);

float4 main(VSOutput In) : SV_TARGET
{
	float3 N = NormalTexture.Sample(samp, In.UV).xyz * 2.0f - 1.0f;
	float3 V = normalize(sceneParam.cameraPos.xyz - In.WorldPos.xyz);
	V = mul(In.TangentBasis, V);
	float3 R = normalize(reflect(V,N));

	float3 light = 0.0f;

	// Lambert
	float4 color = ColorTexture.Sample(samp, In.UV);
	float3 diffuse = DiffuseTexture.Sample(samp, In.UV).rgb;

	for(int i = 0; i < sceneParam.lightIndex; i++)
	{
		float3 L = GetLightDirection(In.WorldPos.xyz, lightBuffer.buffer[i]);
		L = mul(In.TangentBasis, L);
		float3 H = normalize(L + V);
		
		float NoH = saturate(dot(N, H));
		// float LoR = saturate(dot(L, R));

		float3 brdf = BlinnPhongBRDF(materialParam, 0.5f, NoH);
		// float3 brdf = PhongBRDF(materialParam, 0.0f, LoR); // todo: metallic��n����悤��

		light += EvaluateLight(N, In.WorldPos.xyz, lightBuffer.buffer[i]) * brdf * color.rgb;
	}

	// ambient�𑫂�
	light += color.rgb * 0.2f;
	
	// shadow�𑫂�
	// LightViewScreen��Ԃ�
    float4 LVP = mul(shadowParam.LVP, In.WorldPos);
    float3 shadow = CalculateDepthShadow(LVP, ShadowTexture, samp);
    light *= shadow;

	return float4(light.rgb, color.a * materialParam.albedo.a);
}