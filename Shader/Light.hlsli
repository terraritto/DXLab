#ifndef __Light__
#define __Light__

#include "Constant.hlsli"

// ライト関係
struct LightParameter
{
	float4 position; // ライトの位置, wでinvsqrradius
	float4 direction; // ライトの方向, wでintensity
	float4 lightColor; // ライトの色, wでlightangleScale
	float lightAngleOffset;
	int type; // ライトのタイプ
};

struct LightData
{
	LightParameter buffer[MAX_LIGHT_NUM];
};

/* Rim Light */
float RimThreshold(float3 N, float3 V)
{
    return 1.0 - saturate(dot(V, N));
}

/* Point Light */
float GetDistanceAttenuation(float3 lightVec)
{
	float sqrDist = sqrt(dot(lightVec, lightVec));
	float attenuation = 1.0f / (max(sqrDist, MIN_DIST * MIN_DIST));
	return attenuation;
}

float3 EvaluatePointLight(float3 N, float3 worldPos, float3 lightPos, float3 lightColor)
{
	float3 dif = worldPos - lightPos;
	float3 L = normalize(dif);
	float attenuation = GetDistanceAttenuation(dif);
	return saturate(dot(N, L)) * lightColor * attenuation / (4.0f * PI);
}

/* Spot Light */
float GetAngleAttenuation(float3 unnormalizedLightVector, float3 lightDir, float lightAngleScale, float lightAngleOffset)
{
	float cd = dot(lightDir, unnormalizedLightVector);
	float attenuation = saturate(cd * lightAngleScale + lightAngleOffset);
	return attenuation * attenuation;
}

float3 EvaluateSpotLight(float3 N, float3 worldPos, float3 lightPos, float3 lightDir, float3 lightColor, float lightAngleScale, float lightAngleOffset)
{
	float3 unnormalizedLightVector = lightPos - worldPos;
	float3 L = normalize(unnormalizedLightVector);
	float sqrDist = dot(unnormalizedLightVector, unnormalizedLightVector);
	float attenuation = 1.0f / max(sqrDist, MIN_DIST * MIN_DIST);
	attenuation *= GetAngleAttenuation(-L, lightDir, lightAngleScale, lightAngleOffset);
	return saturate(dot(N, L)) * lightColor * attenuation / PI;
}

float SmoothDistanceAttenuation(float squaredDistance, float invSqrAttenuationRadius)
{
    float factor = squaredDistance * invSqrAttenuationRadius;
    float smoothFactor = saturate(1.0 - factor * factor);
    return smoothFactor * smoothFactor;
}

float GetDistanceAttenuation(float3 unnormalizedLightVector, float invSqrAttRadius)
{
	float sqrDist = dot(unnormalizedLightVector, unnormalizedLightVector);
	float att = 1.0f / (max(sqrDist, invSqrAttRadius));
	att *= SmoothDistanceAttenuation(sqrDist, invSqrAttRadius);

	return att;
}

// Legarde 2014
float3 EvaluateSpotLightLegarde(float3 N, float3 worldPos, float3 lightPos, float lightInvRadiusSq, float3 lightDir, float3 lightColor, float lightAngleScale, float lightAngleOffset)
{
	float3 unnormalizedLightVector = lightPos - worldPos;
	float3 L = normalize(unnormalizedLightVector);
	float attenuation = 1.0f;
	attenuation *= GetDistanceAttenuation(unnormalizedLightVector, lightInvRadiusSq);
	attenuation *= GetAngleAttenuation(-L, lightDir, lightAngleScale, lightAngleOffset);
	return saturate(dot(N, L)) * lightColor * attenuation / PI;
}

/* Directional Light */
float3 EvaluateDirectional(float3 N, float3 lightDir, float3 lightColor)
{
	float3 L = normalize(lightDir);
	return saturate(dot(N, L)) * lightColor;
}

/* Lightの方向を取得 */
float3 GetLightDirection(float3 worldPos, LightParameter param)
{
	float3 dir = float3(0.0f, 0.0f, 0.0f);

	switch (param.type)
	{
	case 0:
		dir = param.position.xyz - worldPos;
		break;

	case 1:
		dir = param.direction.xyz;
		break;

	case 2:
		dir = param.direction.xyz;
		break;
	}

	dir = normalize(dir);
	return dir;
}

/* Lightの対応 */
float3 EvaluateLight(float3 N, float3 worldPos, LightParameter param)
{
	float3 light = float3(0.0f, 0.0f, 0.0f);

	if (param.type == 0)
	{
		light += EvaluatePointLight(N, worldPos, param.position.xyz, param.lightColor.xyz);
	}
	else if (param.type == 1)
	{
		light += EvaluateDirectional(N, param.direction.xyz, param.lightColor.xyz);
	}
	else if (param.type == 2)
	{
		// light += EvaluateSpotLight(N, worldPos, param.position.xyz, param.direction.xyz, param.lightColor.xyz, param.lightColor.w, param.lightAngleOffset);
		light += EvaluateSpotLightLegarde(N, worldPos, param.position.xyz, param.position.w, param.direction.xyz, param.lightColor.xyz, param.lightColor.w, param.lightAngleOffset);
	}

	light *= param.direction.w;
	return light;
}

// 球のAreaLight
struct SphereAreaLight
{
    float4 center;
    float4 Le;
    float radius;
};

struct LightSampleInfo
{
    float3 position;
    float3 normal;
    float3 Le;
};

struct SphereAreaLightData
{
    SphereAreaLight buffer[MAX_AREA_NUM];
};

float3 AreaSamplingSphere(float2 uv, const SphereAreaLight light, out LightSampleInfo info)
{
    float z = -2.0f * uv.r + 1.0f;
    float y = sqrt(max(1.0f - z * z, 0.0f)) * sin(PI2 * uv.g);
    float x = sqrt(max(1.0f - z * z, 0.0f)) * cos(PI2 * uv.g);
	
    float3 position = light.center.xyz + light.radius * float3(x, y, z);
    info.position = position;
    info.normal = normalize(float3(x, y, z));
    info.Le = light.Le.xyz;
	
    return position;
}

float SurfaceAreaSphere(const SphereAreaLight light)
{
    return 4.0f * PI * light.radius * light.radius;
}

#endif