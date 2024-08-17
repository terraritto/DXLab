#include "ProcedualMeshTLAS.h"
#include "../../BLAS/ProcedualMesh/ProcedualMeshBLAS.h"

void ProcedualMeshTLAS::InitializeBLAS(ProcedualMeshBLAS* blas)
{
	m_procedualBLAS = blas;
}

const ProcedualMesh* ProcedualMeshTLAS::GetProcedualMesh() const
{
	return m_procedualBLAS->GetProcedualMesh();
}

void ProcedualMeshTLAS::InitializeInstanceDesc()
{
	XMStoreFloat3x4(
		reinterpret_cast<XMFLOAT3X4*>(&m_instanceDesc.Transform),
		m_transform
	);

	m_instanceDesc.InstanceID = m_instanceId;
	m_instanceDesc.InstanceMask = 0xFF;
	m_instanceDesc.InstanceContributionToHitGroupIndex = m_instanceHGIndex;
	m_instanceDesc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
	m_instanceDesc.AccelerationStructure = m_procedualBLAS->GetASResource()->GetGPUVirtualAddress();
}
