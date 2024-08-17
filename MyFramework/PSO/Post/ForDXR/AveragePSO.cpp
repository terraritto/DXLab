#include "AveragePSO.h"

AveragePSO::AveragePSO(std::weak_ptr<DXUTILITY::Device> device)
	: PostPSO(device)
{
	m_constantBuffer.Initialize(m_device, GetCBSize() + 255 & ~255);
	m_constantBuffer.SetupBufferView(m_device);
	m_data = std::make_unique<AverageData>();
}

void AveragePSO::Update(double deltaTime)
{
}

void AveragePSO::WriteCBData()
{
	int index = 0;
	if (auto pDevice = m_device.lock())
	{
		index = pDevice->GetCurrentFrameIndex();
	}

	void* p = m_constantBuffer.Map(index);
	memcpy(p, m_data.get(), GetCBSize());
	m_constantBuffer.Unmap(index);
}

void AveragePSO::Updatespp(int spp)
{
	m_data->m_spp = spp;
}
