#include "DefaultMeshBLAS.h"

void DefaultMeshBLAS::InitializeGeometry()
{
}

void DefaultMeshBLAS::AllocateGeometryResource()
{
	auto mesh = m_mesh.lock();
	if (mesh == nullptr) { return; }

	m_geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	m_geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	auto& triangles = m_geometryDesc.Triangles;
	triangles.VertexBuffer.StartAddress = mesh->m_positionBuffer->GetGPUVirtualAddress();
	triangles.VertexBuffer.StrideInBytes = mesh->m_vertexStride;
	triangles.VertexCount = mesh->m_vertexCount;
	triangles.IndexBuffer = mesh->m_indexBuffer->GetGPUVirtualAddress();
	triangles.IndexCount = mesh->m_indexCount;
	triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
}

void DefaultMeshBLAS::InitializeGeometryDesc()
{
}

void DefaultMeshBLAS::PreBuildAS()
{
	auto device = m_device.lock();
	if (device == nullptr) { return; }

	// AS�̐ݒ�
	auto& inputs = m_buildASDesc.Inputs;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	inputs.NumDescs = 1;
	inputs.pGeometryDescs = &m_geometryDesc;

	// �K�v�ȃ������ʂ����߂�
	device->GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo
	(
		&inputs,
		&m_prebuildDesc
	);
}

void DefaultMeshBLAS::MakeResourceAndConstructAS()
{
	auto device = m_device.lock();
	if (device == nullptr) { return; }

	// scratch buffer�̊m��
	m_scratchResource = device->CreateBuffer
	(
		m_prebuildDesc.ScratchDataSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_HEAP_TYPE_DEFAULT
	);

	// BLAS�p��buffer���m��
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
	m_asResource->SetName(L"Plane-BLAS");

	// AS�̍\�z
	m_buildASDesc.ScratchAccelerationStructureData = m_scratchResource->GetGPUVirtualAddress();
	m_buildASDesc.DestAccelerationStructureData = m_asResource->GetGPUVirtualAddress();

}

void DefaultMeshBLAS::MakeCommandFromList(DXUTILITY::Device::ComPtr<ID3D12GraphicsCommandList4> CommandList)
{
	CommandList->BuildRaytracingAccelerationStructure(
		&m_buildASDesc, 0, nullptr
	);
}
