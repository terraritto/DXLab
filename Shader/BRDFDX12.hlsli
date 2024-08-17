#ifndef __BRDF_DX12__
#define __BRDF_DX12__

#include "CommonDX12.hlsli"
#include "Constant.hlsli"

float3 LambertBRDF(MaterialParameter parameter)
{
	return parameter.albedo.xyz * INVPI;
}

float3 PhongBRDF(MaterialParameter parameter, float metallic, float LoR)
{
	float3 diffuse = LambertBRDF(parameter) * (1.0f - metallic);
	const float3 specColor = parameter.specular.xyz;
	const float shininess = parameter.specular.w;
	const float fr = (shininess + 2.0f) * INVPI2;
	float3 specular = specColor * fr * (shininess <= 0.0f ? 0.0f : pow(LoR, shininess)) * metallic;
	return diffuse + specular;
}

float3 BlinnPhongBRDF(MaterialParameter parameter, float metallic, float NoH)
{
	float3 diffuse = LambertBRDF(parameter) * (1.0f - metallic);
	const float3 specColor = parameter.specular.xyz;
	const float shininess = parameter.specular.w;
	const float fr = (shininess + 8.0f) * INVPI2 / 4.0f;
	float3 specular = specColor * fr * (shininess <= 0.0f ? 0.0f : pow(NoH, shininess)) * metallic;
	return diffuse + specular;
}

#endif