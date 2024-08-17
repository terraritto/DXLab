#pragma once
#include "PolygonMeshBLAS.h"

class CubeBLAS : public PolygonMeshBLAS
{
protected:
	virtual void InitializeGeometry() override;
	virtual void InitializeGeometryDesc() override;
	virtual void PreBuildAS() override;
	virtual void MakeResourceAndConstructAS() override;
	virtual void MakeCommandFromList(DXUTILITY::Device::ComPtr<ID3D12GraphicsCommandList4> CommandList) override;
};
