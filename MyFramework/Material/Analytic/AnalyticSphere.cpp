#include "AnalyticSphere.h"

int AnalyticSphere::GetAnalyticSize()
{
	return sizeof(AnalyticSphereParam);
}

void AnalyticSphere::WriteDataForAnalytic(uint8_t* mapped)
{
	AnalyticSphereParam Data;

	Data.center.x = m_aabbCenter.m128_f32[0];
	Data.center.y = m_aabbCenter.m128_f32[1];
	Data.center.z = m_aabbCenter.m128_f32[2];
	Data.radius = m_radius;

	int size = GetAnalyticSize();

	memcpy(mapped, &Data, size);
	mapped += size;
}
