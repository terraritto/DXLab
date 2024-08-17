#include "AnalyticPlane.h"


int AnalyticPlane::GetAnalyticSize()
{
	return sizeof(AnalyticPlaneParam);
}

void AnalyticPlane::WriteDataForAnalytic(uint8_t* mapped)
{
	AnalyticPlaneParam Data;
	Data.center = m_aabbCenter;
	Data.normal = m_normal;

	int size = GetAnalyticSize();

	memcpy(mapped, &Data, size);
	mapped += size;
}
