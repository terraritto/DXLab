#pragma once
#include <vector>
#include <stdexcept>
#include "../BLASInterface.h"
#include "../../../Geometry/GeometryContainer.h"

class PolygonMeshBLAS : public BLASInterface
{
public:
	// PolygonMesh�̎擾
	PolygonMesh* GetPolygonMesh() const { return m_polygonMesh->GetPolygonMesh(); }

protected:
	virtual void InitializeGeometry() override;
	virtual void AllocateGeometryResource() override;
	virtual void InitializeGeometryDesc() override;
	virtual void PreBuildAS() override;
	virtual void MakeResourceAndConstructAS() override;
	virtual void MakeCommand() override;

protected:
	std::unique_ptr<GeometryContainer> m_polygonMesh;
};
