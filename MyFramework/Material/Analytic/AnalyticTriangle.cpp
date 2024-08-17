#include "AnalyticTriangle.h"

int AnalyticTriangle::GetAnalyticSize()
{
	return sizeof(AnalyticTriangleParam);
}

void AnalyticTriangle::WriteDataForAnalytic(uint8_t* mapped)
{
	AnalyticTriangleParam Data;
	Data.vert[0] = m_vert[0];
	Data.vert[1] = m_vert[1];
	Data.vert[2] = m_vert[2];

	int size = GetAnalyticSize();

	memcpy(mapped, &Data, size);
	mapped += size;
}
