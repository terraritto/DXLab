#pragma once
#include <vector>
#include <stdexcept>
#include "../BLASInterface.h"
#include "../../../Geometry/DefaultMesh.h"

class DefaultMeshBLAS : public BLASInterface
{
public:
	void SetupMesh(std::weak_ptr<DefaultMesh> mesh) { m_mesh = mesh; }

	// PolygonMesh�̎擾
	const std::weak_ptr<DefaultMesh>& GetMesh() { return m_mesh; }

protected:
	virtual void InitializeGeometry() override;
	virtual void AllocateGeometryResource() override;
	virtual void InitializeGeometryDesc() override;
	virtual void PreBuildAS() override;
	virtual void MakeResourceAndConstructAS() override;
	virtual void MakeCommandFromList(DXUTILITY::Device::ComPtr<ID3D12GraphicsCommandList4> CommandList) override;

protected:
	std::weak_ptr<DefaultMesh> m_mesh;
};
