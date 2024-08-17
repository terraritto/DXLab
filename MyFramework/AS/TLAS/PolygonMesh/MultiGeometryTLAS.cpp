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
		
		// idの確認
		auto id = rtState->GetShaderIdentifier(polygon->GetShaderIdentifier().c_str());
		if (id == nullptr) {
			throw std::logic_error("Not found ShaderIdentifier");
		}

		// 1.idの書き込み
		dst += DXUTILITY::WriteShaderIdentifier(dst, id);
		
		// 2.vertex関係
		dst += DXUTILITY::WriteGPUDescriptor(dst, polygonMesh->descriptorIB);
		dst += DXUTILITY::WriteGPUDescriptor(dst, polygonMesh->descriptorVB);

		// Materialがある場合はここも書き込みが行われる
		if (polygon->GetMaterialIndex() != -1)
		{
			// マテリアルの取得
			std::unique_ptr<MaterialBase>& material = MaterialManager::GetInstance().GetMaterial(polygon->GetMaterialIndex());

			// 3.materialのParam
			material->WriteMaterialParamForShaderTable(dst);
		}

		dst = recordStart + hitgroupRecordSize;
	}
}

void MultiGeometryTLAS::InitializeInstanceDesc()
{
	// instanceの初期化は基底
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

	// ASの設定
	auto& inputs = m_buildASDesc.Inputs;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	
	// instance情報
	inputs.NumDescs = UINT(m_instanceDescList.size());
	inputs.InstanceDescs = m_instanceBuffer->GetGPUVirtualAddress();

	// 必要なメモリ量を求める
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
	m_asResource->SetName(L"Scene-TLAS");

	// ASの構築
	m_buildASDesc.ScratchAccelerationStructureData = m_scratchResource->GetGPUVirtualAddress();
	m_buildASDesc.DestAccelerationStructureData = m_asResource->GetGPUVirtualAddress();
	*/
}

void MultiGeometryTLAS::MakeCommand()
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
	D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(m_asResource.Get());
	command->ResourceBarrier(1, &uavBarrier);
	command->Close();

	// command実行
	device->ExecuteCommandList(command);

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
