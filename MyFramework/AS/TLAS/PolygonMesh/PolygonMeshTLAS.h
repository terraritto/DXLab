#pragma once
#include <vector>
#include <stdexcept>
#include "../TLASInterface.h"
#include "../../../Material/MaterialBase.h"
#include "../../../Utility/DXUtility.h"
#include "../../../Geometry/GeometryDefinitions.h"

class PolygonMeshBLAS;

class PolygonMeshTLAS : public TLASInterface
{
public:
	void InitializeBLAS(PolygonMeshBLAS* blas);

	const PolygonMesh* GetPolygonMesh() const;
	
protected:
	virtual void InitializeInstanceDesc() override;

protected:
	PolygonMeshBLAS* m_polygonBLAS;
};