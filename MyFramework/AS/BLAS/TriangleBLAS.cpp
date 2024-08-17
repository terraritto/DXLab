#include "TriangleBLAS.h"

// TODO: InitializeGeometry関係は抜き出す
void TriangleBLAS::InitializeGeometry()
{
	const auto flags = D3D12_RESOURCE_FLAG_NONE;
	const auto heapType = D3D12_HEAP_TYPE_DEFAULT;

	// planeの生成
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

	// 書き込み
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

void TriangleBLAS::MakeResourceAndConstructAS()
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
	m_asResource->SetName(L"Triangle-BLAS");

	// ASの構築
	m_buildASDesc.ScratchAccelerationStructureData = m_scratchResource->GetGPUVirtualAddress();
	m_buildASDesc.DestAccelerationStructureData = m_asResource->GetGPUVirtualAddress();
}

void TriangleBLAS::MakeCommand()
{
	auto device = m_device.lock();
	if (device == nullptr) { return; }

	// command listに積み込む
	auto command = device->CreateCommandList();
	command->BuildRaytracingAccelerationStructure
	(
		&m_buildASDesc,
		0,
		nullptr
	);

	// resource barrierの設定
	D3D12_RESOURCE_BARRIER uavBarrier{};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = m_asResource.Get();
	command->ResourceBarrier(1, &uavBarrier);
	command->Close();

	// command実行
	device->ExecuteCommandList(command);
	device->waitForIdleGpu();
}
