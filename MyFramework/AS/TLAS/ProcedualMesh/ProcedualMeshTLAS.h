#pragma once
#include <vector>
#include <stdexcept>
#include "../TLASInterface.h"
#include "../../../Utility/DXUtility.h"
#include "../../../Geometry/GeometryDefinitions.h"

class ProcedualMeshBLAS;

class ProcedualMeshTLAS : public TLASInterface
{
public:
	void InitializeBLAS(ProcedualMeshBLAS* blas);

	const ProcedualMesh* GetProcedualMesh() const;

protected:
	virtual void InitializeInstanceDesc() override;

protected:
	ProcedualMeshBLAS* m_procedualBLAS;
};