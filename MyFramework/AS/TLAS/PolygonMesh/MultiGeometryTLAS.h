#pragma once
#include <vector>
#include <memory>
#include <stdexcept>
#include "../MultiTLASInterface.h"
#include "PolygonMeshTLAS.h"
#include "../../../Utility/DXUtility.h"
#include "../../../Geometry/GeometryDefinitions.h"

class MultiGeometryTLAS : public MultiTLASInterface
{
public:
	void AddTLAS(std::unique_ptr<PolygonMeshTLAS>& tlas);

	// ShaderTableÇ÷ÇÃèëÇ´çûÇ›óp
	virtual void WriteShaderTable(ComPtr<ID3D12StateObjectProperties>& rtState, uint8_t* dst, UINT hitgroupRecordSize) override;

protected:
	virtual void InitializeInstanceDesc() override;
	virtual void PreBuildAS() override;
	virtual void MakeResourceAndConstructAS() override;
	virtual void MakeCommand() override;

protected:
	std::vector<std::unique_ptr<PolygonMeshTLAS>> m_polygonList;
};