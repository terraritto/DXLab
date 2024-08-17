#pragma once
#include "../ASBase.h"

class BLASInterface : public ASBase
{
public:
	// HitGroup‚ÌIndex‚ÍBLAS–ˆ‚É’è‹`‚µ‚Ä‚â‚é
	void SetHitGroupIndex(const int index) { m_hitGroupIndex = index; }
	const int GetHitGroupIndex() const { return m_hitGroupIndex; }

protected:
	virtual void PreInitializeAS() override
	{
		InitializeGeometry();
		InitializeGeometryDesc();
		AllocateGeometryResource();
	}

protected:
	virtual void InitializeGeometry(){};
	virtual void AllocateGeometryResource(){};
	virtual void InitializeGeometryDesc(){};

protected:	
	// desc
	D3D12_RAYTRACING_GEOMETRY_DESC m_geometryDesc;

	// HitGroup‚ÌIndex‚ğ‚½‚¹‚é
	int m_hitGroupIndex;
};