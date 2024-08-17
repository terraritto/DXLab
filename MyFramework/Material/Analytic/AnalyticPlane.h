#pragma once
#include "AnalyticBase.h"

class AnalyticPlane : public AnalyticBase
{
public:
	AnalyticPlane() : AnalyticBase(){}

	void SetNormal(const XMVECTOR& normal)
	{
		m_normal = normal;
	}

	virtual int GetAnalyticSize() override;
	virtual void WriteDataForAnalytic(uint8_t* mapped) override;

protected:
	struct AnalyticPlaneParam
	{
		XMVECTOR center;
		XMVECTOR normal;
	};

	XMVECTOR m_normal; // ”¼Œa
};