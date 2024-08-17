#pragma once
#include <vector>
#include <stdexcept>
#include "BLASInterface.h"
#include "../../Geometry/GeometryDefinitions.h"

class TriangleBLAS : public BLASInterface
{
protected:
	virtual void InitializeGeometry() override;
	virtual void AllocateGeometryResource() override;
	virtual void InitializeGeometryDesc() override;
	virtual void PreBuildAS() override;
	virtual void MakeResourceAndConstructAS() override;
	virtual void MakeCommand() override;

protected:
	std::vector<PositionVertex> m_vertex;
	size_t m_vertexSize;
	ComPtr<ID3D12Resource> m_vertexBuffer;
};
