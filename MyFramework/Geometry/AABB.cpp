#include "AABB.h"

void AABB::InitializeGeometry(std::weak_ptr<DXUTILITY::Device> device)
{
	const auto flags = D3D12_RESOURCE_FLAG_NONE;
	const auto heapType = D3D12_HEAP_TYPE_DEFAULT;

	// AABB‚ð’è‹`
	D3D12_RAYTRACING_AABB AABBData;
	AABBData.MinX = -1.0f * m_scale;
	AABBData.MinY = -1.0f * m_scale;
	AABBData.MinZ = -1.0f * m_scale;
	AABBData.MaxX = 1.0f * m_scale;
	AABBData.MaxY = 1.0f * m_scale;
	AABBData.MaxZ = 1.0f * m_scale;

	m_procedualMesh->aabbBuffer = DXUTILITY::CreateBuffer(device, sizeof(AABBData), &AABBData, heapType, flags, L"meshAABB");
}
