#include "AnalyticBase.h"

UINT AnalyticBase::WriteMaterialParamForShaderTable(uint8_t* dst, int index)
{
	int size = MaterialBase::WriteMaterialParamForShaderTable(dst, index);

	dst += size;

	// Analytic—p‚ÌCB‚à‘‚«‚Ş‚æ‚¤‚É
	if (m_analyticCB.Get(index) == nullptr)
	{
		return 0;
	}

	// ‘¶İ‚·‚éê‡‚Í‘‚«‚İ
	return DXUTILITY::WriteGpuResourceAddr(dst, m_analyticCB.Get(index)) + size;
}

void AnalyticBase::AllocateConstantBuffer(std::weak_ptr<DXUTILITY::Device> device)
{
	MaterialBase::AllocateConstantBuffer(device);

	UINT TotalSize = GetAnalyticSize();

	// ¶¬
	m_analyticCB.Initialize(device, TotalSize, L"Analytic");
}
