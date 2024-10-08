#include "MaterialBase.h"
#include <stdexcept>

void MaterialBase::AllocateConstantBuffer(std::weak_ptr<DXUTILITY::Device> device)
{
	UINT TotalSize = GetTotalBufferSize();

	// 生成
	m_materialCB.Initialize(device, TotalSize, L"Material");
}

void MaterialBase::AllocateBuffer(std::weak_ptr<DXUTILITY::Device> device)
{
	UINT TotalSize = GetTotalBufferSize();

	// 生成
	m_materialDescriptorCB.Initialize(device, TotalSize, L"Material");
	m_materialDescriptorCB.SetupBufferView(device);
}

UINT MaterialBase::WriteMaterialParamForShaderTable(uint8_t* dst, int index)
{
	if (m_materialCB.Get(index) == nullptr)
	{
		return 0;
	}

	// 存在する場合は書き込み
	return DXUTILITY::WriteGpuResourceAddr(dst, m_materialCB.Get(index));
}

D3D12_GPU_DESCRIPTOR_HANDLE MaterialBase::GetDescriptor(int index)
{
	return m_materialDescriptorCB.GetDescriptor(index).m_gpuHandle;
}
