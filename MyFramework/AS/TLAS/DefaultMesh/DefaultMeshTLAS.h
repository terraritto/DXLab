#pragma once
#include <vector>
#include <stdexcept>
#include "../TLASInterface.h"
#include "../../../Material/MaterialBase.h"
#include "../../../Utility/DXUtility.h"
#include "../../../Geometry/GeometryDefinitions.h"

class DefaultMeshBLAS;
struct DefaultMesh;

class DefaultMeshTLAS : public TLASInterface
{
public:
	void InitializeBLAS(DefaultMeshBLAS* blas);

	const std::weak_ptr<DefaultMesh>& GetMesh() const;

protected:
	virtual void InitializeInstanceDesc() override;

protected:
	DefaultMeshBLAS* m_meshBLAS;
};