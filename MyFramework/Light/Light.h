#pragma once
#include "../../MyFramework/Utility/TypeUtility.h"
#include <assimp/scene.h>

enum class LightType
{
	Directional,
	Point,
	Spot,

	Invalid
};

int ConvertType(aiLightSourceType type);

struct Light
{
public:
	XMVECTOR m_position; // Lightの位置(PointLight用)
	XMVECTOR m_direction; // Lightの方向(Directional用)
	XMVECTOR m_color;	// Lightのカラー
	float m_radius;
	float m_innerAngle, m_outerAngle;

	float m_intensity = 1.0f; // Lightの強さ(Defaultは1)

	std::string m_name;
	int m_type = -1; // Lightのタイプ
};

struct WriteLightData
{
	XMVECTOR m_position; // Lightの位置(PointLight用)
	XMVECTOR m_direction; // Lightの方向(Directional用)
	XMVECTOR m_color;
	float m_lightAngleOffset;
	int m_type = -1; // Lightのタイプ

	void Setup(Light& light)
	{
		float cosInnerAngle = std::cos(light.m_innerAngle);
		float cosOuterAngle = std::cos(light.m_outerAngle);

		this->m_position = light.m_position;
		this->m_position.m128_f32[3] = 1.0f / (light.m_radius * light.m_radius);
		this->m_direction = light.m_direction;
		this->m_direction.m128_f32[3] = light.m_intensity;
		this->m_color = light.m_color;
		this->m_color.m128_f32[3] = 1.0f / DirectX::XMMax(0.001f, cosInnerAngle - cosOuterAngle);
		this->m_lightAngleOffset = -cosInnerAngle * this->m_color.m128_f32[3];
		this->m_type = light.m_type;
	}
};

struct SphereAreaLightData
{
	XMVECTOR m_position;
	XMVECTOR m_emission;
	float m_radius;
};

struct WriteSphereAreaLightData
{
	XMVECTOR m_position;
	XMVECTOR m_emission;
	float m_radius;

	void Setup(SphereAreaLightData& light)
	{
		this->m_position = light.m_position;
		this->m_emission = light.m_emission;
		this->m_radius = light.m_radius;
	}
};

std::string ConvertLightTypeString(Light& light);
