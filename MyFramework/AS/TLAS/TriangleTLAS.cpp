#include "TriangleTLAS.h"
#include "../BLAS/TriangleBLAS.h"
#include "PolygonMesh\MultiGeometryTLAS.h"

void TriangleTLAS::InitializeBLAS(TriangleBLAS* blas)
{
	triangleBLAS = blas;
}

void TriangleTLAS::InitializeInstanceDesc()
{
	XMStoreFloat3x4(
		reinterpret_cast<XMFLOAT3X4*>(&m_instanceDesc.Transform),
		DirectX::XMMatrixIdentity()
	);
	m_instanceDesc.InstanceID = 0;
	m_instanceDesc.InstanceMask = 0xFF;
	m_instanceDesc.InstanceContributionToHitGroupIndex = 0;
	m_instanceDesc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
	m_instanceDesc.AccelerationStructure = triangleBLAS->GetASResource()->GetGPUVirtualAddress();
}

void TriangleTLAS::PreBuildAS()
{
	/*
	auto device = m_device.lock();
	if (device == nullptr) { return; }

	// ASの設定
	auto& inputs = m_buildASDesc.Inputs;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	inputs.NumDescs = 1;

	// 必要なメモリ量を求める
	device->GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo
	(
		&inputs,
		&m_prebuildDesc
	);
	*/
}

void TriangleTLAS::MakeResourceAndConstructAS()
{
	/*
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

	// TLAS用のbufferを確保
	m_asResource = device->CreateBuffer
	(
		m_prebuildDesc.ResultDataMaxSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		D3D12_HEAP_TYPE_DEFAULT
	);

	if (m_asResource == nullptr || m_scratchResource == nullptr)
	{
		throw std::runtime_error("TLAS Creation is failed.");
	}
	m_asResource->SetName(L"Triangle-TLAS");

	// ASの構築
	m_buildASDesc.Inputs.InstanceDescs = m_instanceBuffer->GetGPUVirtualAddress();
	m_buildASDesc.ScratchAccelerationStructureData = m_scratchResource->GetGPUVirtualAddress();
	m_buildASDesc.DestAccelerationStructureData = m_asResource->GetGPUVirtualAddress();
	*/
}

void TriangleTLAS::MakeCommand()
{
	/*
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

	// descriptorの準備
	m_tlasDescriptor = device->AllocateDescriptor();

	// SRV作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.RaytracingAccelerationStructure.Location = m_asResource->GetGPUVirtualAddress();
	device->GetDevice()->CreateShaderResourceView(nullptr, &srvDesc, m_tlasDescriptor.m_cpuHandle);

	// 待ち
	device->waitForIdleGpu();
	*/
}
