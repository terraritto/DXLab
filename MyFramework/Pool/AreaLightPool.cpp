#include "AreaLightPool.h"

AreaLightPool::~AreaLightPool()
{
}

void AreaLightPool::RegisterAreaSphereLight(std::unique_ptr<SphereAreaLightData>&& light)
{
	m_areaLightList.emplace_back(std::move(light));
}

void AreaLightPool::Initialize(std::weak_ptr<DXUTILITY::Device> device)
{
	auto pDevice = device.lock();
	if (pDevice == nullptr) { return; }

	auto totalSize = sizeof(WriteSphereAreaLightData) * 100;

	// AreaLightÇÃópà”
	m_areaLightCB.Initialize(pDevice, totalSize);
	m_areaLightCB.SetupBufferView(pDevice);

	// èëÇ´çûÇÒÇ≈Ç®Ç≠
	const UINT count = pDevice->BACK_BUFFER_COUNT;

	for (auto i = 0; i < count; i++)
	{
		UpdateData(i);
	}
}

void AreaLightPool::Uninitialize(std::weak_ptr<DXUTILITY::Device> device)
{
	auto pDevice = device.lock();
	if (pDevice == nullptr) { return; }

	m_areaLightCB.UnInitialize(pDevice);
}

void AreaLightPool::UpdateData(int index)
{
	auto totalSize = sizeof(WriteSphereAreaLightData) * 100;
	std::array<WriteSphereAreaLightData, 100> lightData;

	// lightÇÃçXêV
	void* p = m_areaLightCB.Map(index);

	if (p == nullptr) { return; }

	for (int i = 0; i < m_areaLightList.size(); i++)
	{
		lightData[i].Setup(*m_areaLightList[i]);
	}

	memcpy(p, lightData.data(), totalSize);

	m_areaLightCB.Unmap(index);
}

DXUTILITY::Descriptor AreaLightPool::GetDescriptor(int frameIndex)
{
	return m_areaLightCB.GetDescriptor(frameIndex);
}
