#include "AnalyticBase.h"

UINT AnalyticBase::WriteMaterialParamForShaderTable(uint8_t* dst, int index)
{
	int size = MaterialBase::WriteMaterialParamForShaderTable(dst, index);

	dst += size;

	// Analytic用のCBも書き込むように
	if (m_analyticCB.Get(index) == nullptr)
	{
		return 0;
	}

	// 存在する場合は書き込み
	return DXUTILITY::WriteGpuResourceAddr(dst, m_analyticCB.Get(index)) + size;
}

void AnalyticBase::AllocateConstantBuffer(std::weak_ptr<DXUTILITY::Device> device)
{
	MaterialBase::AllocateConstantBuffer(device);

	UINT TotalSize = GetAnalyticSize();

	// 生成
	m_analyticCB.Initialize(device, TotalSize, L"Analytic");
}
