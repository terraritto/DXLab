#include "../../../../Shader/BRDFDX12.hlsli"
#include "../../../../Shader/CommonDX12.hlsli"
#include "../../../../Shader/Constant.hlsli"
#include "../../../../Shader/Light.hlsli"
#include "../../../../Shader/Shadow/ShadowCommon.hlsli"
#include "../../../../Shader/DXR/BRDF/BRDFData.hlsli"

Texture2D ColorTexture : register(t0);
Texture2D DiffuseTexture : register(t1);
Texture2D NormalTexture : register(t2);
Texture2D ShadowTexture : register(t3);
SamplerState samp : register(s0);
ConstantBuffer<HemisphereLightShaderParameter> sceneParam : register(b0);
ConstantBuffer<SceneShadowParameter> shadowParam : register(b1);
ConstantBuffer<EncodeData> mat : register(b2);
ConstantBuffer<LightData> lightBuffer : register(b3);

float4 main(VSOutput In) : SV_TARGET
{
	float3 N = NormalTexture.Sample(samp, In.UV).xyz * 2.0f - 1.0f;
	N = mul(In.TangentBasis, N);
	float3 V = normalize(sceneParam.cameraPos.xyz - In.WorldPos.xyz);
	V = mul(In.TangentBasis, V);
	float3 R = normalize(reflect(V, N));

	float3 light = 0.0f;

	// Lambert
	float4 color = ColorTexture.Sample(samp, In.UV);
	float3 diffuse = DiffuseTexture.Sample(samp, In.UV).rgb;

	MaterialParameter materialParam;
	Decode(mat, materialParam);

	for (int i = 0; i < sceneParam.lightIndex; i++)
	{
		float3 L = GetLightDirection(In.WorldPos.xyz, lightBuffer.buffer[i]);
		L = mul(In.TangentBasis, L);
		float3 H = normalize(L + V);

		float NoH = saturate(dot(N, H));
		float LoR = saturate(dot(L, R));

		float3 brdf = BlinnPhongBRDF(materialParam, 0.5f, NoH);
		light += EvaluateLight(N, In.WorldPos.xyz, lightBuffer.buffer[i]) * brdf * diffuse.rgb;
	}

	// ambient‚ð‘«‚·
	// light = color.rgb * 0.8f;
	light = CalculateHemisphereLight(N, sceneParam);

	// shadow‚ð‘«‚·
	// LightViewScreen‹óŠÔ‚Ö
	float4 LVP = mul(shadowParam.LVP, In.WorldPos);
	float3 shadow = CalculatePCFShadow(LVP, shadowParam.offset, ShadowTexture, samp);
	//light *= shadow;

	return float4(light.xyz, 1.0f);
}