#include "TriangleBLAS.h"

// TODO: InitializeGeometry�֌W�͔����o��
void TriangleBLAS::InitializeGeometry()
{
	const auto flags = D3D12_RESOURCE_FLAG_NONE;
	const auto heapType = D3D12_HEAP_TYPE_DEFAULT;

	// plane�̐���
	std::vector<VertexPNC> vertices;
	std::vector<UINT> indices;
}

void TriangleBLAS::AllocateGeometryResource()
{
	auto device = m_device.lock();
	if (device == nullptr) { return; }

	m_vertexBuffer = device->CreateBuffer
	(
		m_vertexSize,
		D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		D3D12_HEAP_TYPE_UPLOAD
	);
	if (m_vertexBuffer == nullptr)
	{
		throw std::runtime_error("Creating VertexBuffer is failed");
	}

	// ��������
	device->WriteToHostVisibleMemory(m_vertexBuffer, m_vertex.data(), m_vertexSize);
}

void TriangleBLAS::InitializeGeometryDesc()
{
	m_geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	m_geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
	m_geometryDesc.Triangles.VertexBuffer.StartAddress = m_vertexBuffer->GetGPUVirtualAddress();
	m_geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(XMFLOAT3);
	m_geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	m_geometryDesc.Triangles.VertexCount = m_vertex.size();
}

void TriangleBLAS::PreBuildAS()
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

void TriangleBLAS::MakeResourceAndConstructAS()
{
	auto device = m_device.lock();
	if (device == nullptr) { return; }

	// scratch buffer�̊m��
	m_scratchResource = device->CreateBuffer
	(
		m_prebuildDesc.ScratchDataSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
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
	m_asResource->SetName(L"Triangle-BLAS");

	// AS�̍\�z
	m_buildASDesc.ScratchAccelerationStructureData = m_scratchResource->GetGPUVirtualAddress();
	m_buildASDesc.DestAccelerationStructureData = m_asResource->GetGPUVirtualAddress();
}

void TriangleBLAS::MakeCommand()
{
	auto device = m_device.lock();
	if (device == nullptr) { return; }

	// command list�ɐςݍ���
	auto command = device->CreateCommandList();
	command->BuildRaytracingAccelerationStructure
	(
		&m_buildASDesc,
		0,
		nullptr
	);

	// resource barrier�̐ݒ�
	D3D12_RESOURCE_BARRIER uavBarrier{};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = m_asResource.Get();
	command->ResourceBarrier(1, &uavBarrier);
	command->Close();

	// command���s
	device->ExecuteCommandList(command);
	device->waitForIdleGpu();
}
