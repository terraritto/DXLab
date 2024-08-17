#include "PlaneWithUVBLAS.h"
#include "../../../Geometry/PlaneWithUV.h"

void PlaneWithUVBLAS::InitializeGeometry()
{
	m_polygonMesh = std::make_unique<PlaneWithUV>();
}

void PlaneWithUVBLAS::InitializeGeometryDesc()
{
	m_polygonMesh->InitializeGeometry(m_device);
}

void PlaneWithUVBLAS::PreBuildAS()
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

void PlaneWithUVBLAS::MakeResourceAndConstructAS()
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
	m_asResource->SetName(L"Plane-BLAS");

	// AS�̍\�z
	m_buildASDesc.ScratchAccelerationStructureData = m_scratchResource->GetGPUVirtualAddress();
	m_buildASDesc.DestAccelerationStructureData = m_asResource->GetGPUVirtualAddress();
}

void PlaneWithUVBLAS::MakeCommandFromList(DXUTILITY::Device::ComPtr<ID3D12GraphicsCommandList4> CommandList)
{
	CommandList->BuildRaytracingAccelerationStructure(
		&m_buildASDesc, 0, nullptr
	);
}
