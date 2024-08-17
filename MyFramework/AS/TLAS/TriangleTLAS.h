#pragma once
#include <vector>
#include <stdexcept>
#include "TLASInterface.h"
#include "../../Utility/DXUtility.h"
#include "../../Geometry/GeometryDefinitions.h"

class TriangleBLAS;

class TriangleTLAS : public TLASInterface
{
public:
	void InitializeBLAS(TriangleBLAS* blas);

protected:
	virtual void InitializeInstanceDesc() override;
	virtual void PreBuildAS() override;
	virtual void MakeResourceAndConstructAS() override;
	virtual void MakeCommand() override;

protected:
	TriangleBLAS* triangleBLAS;
	ComPtr<ID3D12Resource> m_instanceBuffer;
};