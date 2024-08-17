#include "MultiTLASPool.h"
#include "../../include/d3dx12.h"

MultiTLASPool::~MultiTLASPool()
{
}

void MultiTLASPool::Setup()
{
	m_meshMultiTLAS = std::make_unique<MultiDefaultMeshTLAS>();
	m_procedualMultiTLAS = std::make_unique<MultiProcedualTLAS>();
}

void MultiTLASPool::ConstructTLAS(std::weak_ptr<DXUTILITY::Device> device)
{
	// InstanceDescを初期化していく
	m_procedualMultiTLAS->Initialize(device);
	m_meshMultiTLAS->Initialize(device);

	// bufferの構築
	auto multiTLASDescList = m_procedualMultiTLAS->GetInstanceDescList();
	auto meshTLASDescList = m_meshMultiTLAS->GetInstanceDescList();
	multiTLASDescList.insert(multiTLASDescList.end(), meshTLASDescList.begin(), meshTLASDescList.end());

	// ここのサイズを記録
	m_tlasSize = static_cast<int>(multiTLASDescList.size());

	for (auto& buffer : m_instanceBuffers)
	{
		buffer = DXUTILITY::CreateBuffer(
			device,
			multiTLASDescList.size() * sizeof(D3D12_RAYTRACING_INSTANCE_DESC),
			multiTLASDescList.data(),
			D3D12_HEAP_TYPE_UPLOAD,
			D3D12_RESOURCE_FLAG_NONE,
			L"InstanceDescBuffer"
		);
	}

	auto pDevice = device.lock();
	if (pDevice == nullptr) { return; }

	// ASの設定
	auto& inputs = m_buildASDesc.Inputs;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;

	// instance情報
	inputs.NumDescs = UINT(multiTLASDescList.size());
	inputs.InstanceDescs = m_instanceBuffers[0]->GetGPUVirtualAddress();

	// 必要なメモリ量を求める
	pDevice->GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo
	(
		&inputs,
		&m_prebuildDesc
	);

	// scratch bufferの確保
	m_scratchResource = pDevice->CreateBuffer
	(
		m_prebuildDesc.ScratchDataSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_HEAP_TYPE_DEFAULT
	);

	// TLAS用のbufferを確保
	m_asResource = pDevice->CreateBuffer
	(
		m_prebuildDesc.ResultDataMaxSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		D3D12_HEAP_TYPE_DEFAULT
	);

	if (m_buildASDesc.Inputs.Flags & D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE)
	{
		// Updeteの確保
		m_updateResource = pDevice->CreateBuffer
		(
			m_prebuildDesc.UpdateScratchDataSizeInBytes,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_HEAP_TYPE_DEFAULT
		);

		if (m_updateResource == nullptr)
		{
			throw std::runtime_error("Update TLAS Resource Creation is failed.");
		}
	}

	if (m_asResource == nullptr || m_scratchResource == nullptr)
	{
		throw std::runtime_error("TLAS Creation is failed.");
	}
	m_asResource->SetName(L"Scene-TLAS");

	// ASの構築
	m_buildASDesc.ScratchAccelerationStructureData = m_scratchResource->GetGPUVirtualAddress();
	m_buildASDesc.DestAccelerationStructureData = m_asResource->GetGPUVirtualAddress();

	// descriptorの準備
	m_tlasDescriptor = pDevice->AllocateDescriptor();

	// SRV作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.RaytracingAccelerationStructure.Location = m_asResource->GetGPUVirtualAddress();
	pDevice->GetDevice()->CreateShaderResourceView(nullptr, &srvDesc, m_tlasDescriptor.m_cpuHandle);

	// command listに積み込む
	auto command = pDevice->CreateCommandList();
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
	pDevice->ExecuteCommandList(command);

	// 待ち
	pDevice->waitForIdleGpu();
}

// 書き込み順はProcedual->Meshの順
void MultiTLASPool::WriteShaderTable(ComPtr<ID3D12StateObjectProperties>& rtState, uint8_t* dst, UINT hitgroupRecordSize)
{
	m_procedualMultiTLAS->WriteShaderTable(rtState, dst, hitgroupRecordSize);
	m_meshMultiTLAS->WriteShaderTable(rtState, dst, hitgroupRecordSize);
}

void MultiTLASPool::WriteShaderTableFromIndex(ComPtr<ID3D12StateObjectProperties>& rtState, uint8_t* dst, UINT hitgroupRecordSize, UINT index)
{
	m_procedualMultiTLAS->WriteShaderTableFromIndex(rtState, dst, hitgroupRecordSize, index);
	dst = dst + m_procedualMultiTLAS->GetInstanceDescList().size() * hitgroupRecordSize;
	m_meshMultiTLAS->WriteShaderTableFromIndex(rtState, dst, hitgroupRecordSize, index);
}

void MultiTLASPool::AddTLAS(std::unique_ptr<DefaultMeshTLAS>& tlas)
{
	m_meshMultiTLAS->AddTLAS(tlas);
}

void MultiTLASPool::AddTLAS(std::unique_ptr<ProcedualMeshTLAS>& tlas)
{
	m_procedualMultiTLAS->AddTLAS(tlas);
}
