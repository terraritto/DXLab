#include "DefaultMeshTLAS.h"
#include "../../BLAS/DefaultMesh/DefaultMeshBLAS.h"

void DefaultMeshTLAS::InitializeBLAS(DefaultMeshBLAS* blas)
{
	m_meshBLAS = blas;
}

const std::weak_ptr<DefaultMesh>& DefaultMeshTLAS::GetMesh() const
{
	return m_meshBLAS->GetMesh();
}

void DefaultMeshTLAS::InitializeInstanceDesc()
{
	XMStoreFloat3x4(
		reinterpret_cast<XMFLOAT3X4*>(&m_instanceDesc.Transform),
		m_transform
	);

	m_instanceDesc.InstanceID = m_instanceId;
	m_instanceDesc.InstanceMask = 0xFF;
	m_instanceDesc.InstanceContributionToHitGroupIndex = m_instanceHGIndex;
	m_instanceDesc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
	m_instanceDesc.AccelerationStructure = m_meshBLAS->GetASResource()->GetGPUVirtualAddress();
}
