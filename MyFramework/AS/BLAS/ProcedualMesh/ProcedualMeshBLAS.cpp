#include "ProcedualMeshBLAS.h"
#include "../../../Geometry/AABB.h"

void ProcedualMeshBLAS::InitializeGeometry()
{
	m_procedualMesh = std::make_unique<AABB>();
	m_procedualMesh->SetScale(m_scale);
}

void ProcedualMeshBLAS::AllocateGeometryResource()
{
	m_geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
	m_geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	ProcedualMesh* mesh = m_procedualMesh->GetProcedualMesh();

	// AABBの設定
	auto& aabbs = m_geometryDesc.AABBs;
	aabbs.AABBCount = 1;
	aabbs.AABBs.StartAddress = mesh->aabbBuffer->GetGPUVirtualAddress();
	aabbs.AABBs.StrideInBytes = sizeof(D3D12_RAYTRACING_AABB);
}

void ProcedualMeshBLAS::InitializeGeometryDesc()
{
	m_procedualMesh->InitializeGeometry(m_device);
}

void ProcedualMeshBLAS::PreBuildAS()
{
	auto device = m_device.lock();
	if (device == nullptr) { return; }

	// ASの設定
	auto& inputs = m_buildASDesc.Inputs;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	inputs.NumDescs = 1;
	inputs.pGeometryDescs = &m_geometryDesc;

	// 必要なメモリ量を求める
	device->GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo
	(
		&inputs,
		&m_prebuildDesc
	);
}

void ProcedualMeshBLAS::MakeResourceAndConstructAS()
{
	auto device = m_device.lock();
	if (device == nullptr) { return; }

	// scratch bufferの確保
	m_scratchResource = device->CreateBuffer
	(
		m_prebuildDesc.ScratchDataSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_HEAP_TYPE_DEFAULT
	);

	// BLAS用のbufferを確保
	m_asResource = device->CreateBuffer
	(
		m_prebuildDesc.ResultDataMaxSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		D3D12_HEAP_TYPE_DEFAULT
	);

	if (m_asResource == nullptr || m_scratchResource == nullptr)
	{
		throw std::runtime_error("BLAS Creation is failed.");
	}
	m_asResource->SetName(L"AABB-BLAS");

	// ASの構築
	m_buildASDesc.ScratchAccelerationStructureData = m_scratchResource->GetGPUVirtualAddress();
	m_buildASDesc.DestAccelerationStructureData = m_asResource->GetGPUVirtualAddress();
}

void ProcedualMeshBLAS::MakeCommand()
{
}

void ProcedualMeshBLAS::MakeCommandFromList(DXUTILITY::Device::ComPtr<ID3D12GraphicsCommandList4> CommandList)
{
	CommandList->BuildRaytracingAccelerationStructure(&m_buildASDesc, 0, nullptr);
}
