#pragma once
#include "AnalyticBase.h"

class AnalyticTriangle : public AnalyticBase
{
public:
	AnalyticTriangle() : AnalyticBase(){}

	void SetVert(const std::array<XMVECTOR, 3>& vert)
	{
		m_vert = vert;
	}

	void SetVert(int index, XMVECTOR vert)
	{
		m_vert[index] = vert;
	}

	virtual int GetAnalyticSize() override;
	virtual void WriteDataForAnalytic(uint8_t* mapped) override;

protected:
	struct AnalyticTriangleParam
	{
		XMVECTOR vert[3];
	};

	std::array<XMVECTOR, 3> m_vert;
};