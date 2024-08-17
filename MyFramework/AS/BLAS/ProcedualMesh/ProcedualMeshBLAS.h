#pragma once
#include <vector>
#include <stdexcept>
#include "../BLASInterface.h"
#include "../../../Geometry/ProcedualContainer.h"

class ProcedualMeshBLAS : public BLASInterface
{
public:
	// PolygonMesh�̎擾
	ProcedualMesh* GetProcedualMesh() const { return m_procedualMesh->GetProcedualMesh(); }

	void SetScale(const float scale) { m_scale = scale; }

protected:
	virtual void InitializeGeometry() override;
	virtual void AllocateGeometryResource() override;
	virtual void InitializeGeometryDesc() override;
	virtual void PreBuildAS() override;
	virtual void MakeResourceAndConstructAS() override;
	virtual void MakeCommand() override;
	virtual void MakeCommandFromList(DXUTILITY::Device::ComPtr<ID3D12GraphicsCommandList4> CommandList) override;

protected:
	std::unique_ptr<ProcedualContainer> m_procedualMesh;

	float m_scale = 1.0f;
};
