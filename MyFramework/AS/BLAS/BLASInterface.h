#pragma once
#include "../ASBase.h"

class BLASInterface : public ASBase
{
public:
	// HitGroup��Index��BLAS���ɒ�`���Ă��
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

	// HitGroup��Index����������
	int m_hitGroupIndex;
};