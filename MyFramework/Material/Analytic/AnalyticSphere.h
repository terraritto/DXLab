#pragma once
#include "AnalyticBase.h"

class AnalyticSphere : public AnalyticBase
{
public:
	AnalyticSphere() : AnalyticBase(){}

	void SetRadius(const float& radius)
	{
		m_radius = radius;
	}

	virtual int GetAnalyticSize() override;
	virtual void WriteDataForAnalytic(uint8_t* mapped) override;

protected:
	struct AnalyticSphereParam
	{
		XMFLOAT3 center;
		float radius;
	};

	float m_radius; // ”¼Œa
};