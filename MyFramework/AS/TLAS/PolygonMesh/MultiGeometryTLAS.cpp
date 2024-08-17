#include "MultiGeometryTLAS.h"
#include "../../../include/d3dx12.h"
#include "PolygonMeshTLAS.h"
#include "../../../Material/MaterialManager.h"

void MultiGeometryTLAS::AddTLAS(std::unique_ptr<PolygonMeshTLAS>& tlas)
{
	m_polygonList.push_back(std::move(tlas));
}

void MultiGeometryTLAS::WriteShaderTable(ComPtr<ID3D12StateObjectProperties>& rtState, uint8_t* dst, UINT hitgroupRecordSize)
{
	for (auto& polygon : m_polygonList)
	{
		auto recordStart = dst;
		const auto& polygonMesh = polygon->GetPolygonMesh();
		
		// id�̊m�F
		auto id = rtState->GetShaderIdentifier(polygon->GetShaderIdentifier().c_str());
		if (id == nullptr) {
			throw std::logic_error("Not found ShaderIdentifier");
		}

		// 1.id�̏�������
		dst += DXUTILITY::WriteShaderIdentifier(dst, id);
		
		// 2.vertex�֌W
		dst += DXUTILITY::WriteGPUDescriptor(dst, polygonMesh->descriptorIB);
		dst += DXUTILITY::WriteGPUDescriptor(dst, polygonMesh->descriptorVB);

		// Material������ꍇ�͂������������݂��s����
		if (polygon->GetMaterialIndex() != -1)
		{
			// �}�e���A���̎擾
			std::unique_ptr<MaterialBase>& material = MaterialManager::GetInstance().GetMaterial(polygon->GetMaterialIndex());

			// 3.material��Param
			material->WriteMaterialParamForShaderTable(dst);
		}

		dst = recordStart + hitgroupRecordSize;
	}
}

void MultiGeometryTLAS::InitializeInstanceDesc()
{
	// instance�̏������͊��
	m_instanceDescList.resize(m_polygonList.size());
	for (int i = 0; i < m_polygonList.size(); i++)
	{
		m_polygonList[i]->Initialize(m_device);
		m_instanceDescList[i] = m_polygonList[i]->GetInstanceDesc();
	}
}

void MultiGeometryTLAS::PreBuildAS()
{
	/*
	auto device = m_device.lock();
	if (device == nullptr) { return; }

	// AS�̐ݒ�
	auto& inputs = m_buildASDesc.Inputs;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	
	// instance���
	inputs.NumDescs = UINT(m_instanceDescList.size());
	inputs.InstanceDescs = m_instanceBuffer->GetGPUVirtualAddress();

	// �K�v�ȃ������ʂ����߂�
	device->GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo
	(
		&inputs,
		&m_prebuildDesc
	);
	*/
}

void MultiGeometryTLAS::MakeResourceAndConstructAS()
{
	/*
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

	// TLAS�p��buffer���m��
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
	m_asResource->SetName(L"Scene-TLAS");

	// AS�̍\�z
	m_buildASDesc.ScratchAccelerationStructureData = m_scratchResource->GetGPUVirtualAddress();
	m_buildASDesc.DestAccelerationStructureData = m_asResource->GetGPUVirtualAddress();
	*/
}

void MultiGeometryTLAS::MakeCommand()
{
	/*
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
	D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(m_asResource.Get());
	command->ResourceBarrier(1, &uavBarrier);
	command->Close();

	// command���s
	device->ExecuteCommandList(command);

	// descriptor�̏���
	m_tlasDescriptor = device->AllocateDescriptor();

	// SRV�쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.RaytracingAccelerationStructure.Location = m_asResource->GetGPUVirtualAddress();
	device->GetDevice()->CreateShaderResourceView(nullptr, &srvDesc, m_tlasDescriptor.m_cpuHandle);

	// �҂�
	device->waitForIdleGpu();
	*/
}
