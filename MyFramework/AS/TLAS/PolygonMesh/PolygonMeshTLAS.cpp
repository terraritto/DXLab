#include "PolygonMeshTLAS.h"
#include "../../BLAS/PolygonMesh/PolygonMeshBLAS.h"

void PolygonMeshTLAS::InitializeBLAS(PolygonMeshBLAS* blas)
{
	m_polygonBLAS = blas;
}

const PolygonMesh* PolygonMeshTLAS::GetPolygonMesh() const
{
	return m_polygonBLAS->GetPolygonMesh();
}

void PolygonMeshTLAS::InitializeInstanceDesc()
{	
	XMStoreFloat3x4(
		reinterpret_cast<XMFLOAT3X4*>(&m_instanceDesc.Transform),
		m_transform
	);

	m_instanceDesc.InstanceID = m_instanceId;
	m_instanceDesc.InstanceMask = 0xFF;
	m_instanceDesc.InstanceContributionToHitGroupIndex = m_instanceHGIndex;
	m_instanceDesc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
	m_instanceDesc.AccelerationStructure = m_polygonBLAS->GetASResource()->GetGPUVirtualAddress();
}
