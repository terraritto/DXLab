#include "LightPool.h"

LightPool::~LightPool()
{
}

void LightPool::RegisterLight(std::unique_ptr<Light>&& Light)
{
	m_lightList.emplace_back(std::move(Light));
}

void LightPool::Initialize(std::weak_ptr<DXUTILITY::Device> device)
{
	auto pDevice = device.lock();
	if (pDevice == nullptr) { return; }

	auto totalSize = sizeof(WriteLightData) * 100;

	// バッファの用意
	m_lightCB.Initialize(pDevice, totalSize);
	m_lightCB.SetupBufferView(pDevice);

	// 書き込んでおく
	const UINT count = pDevice->BACK_BUFFER_COUNT;

	for (auto i = 0; i < count; i++)
	{
		UpdateData(i);
	}
}

void LightPool::Uninitialize(std::weak_ptr<DXUTILITY::Device> device)
{
	auto pDevice = device.lock();
	if (pDevice == nullptr) { return; }

	m_lightCB.UnInitialize(pDevice);
}

void LightPool::UpdateData(int index)
{
	auto totalSize = sizeof(WriteLightData) * 100;
	std::array<WriteLightData, 100> lightData;

	// lightの更新
	void* p = m_lightCB.Map(index);

	if (p == nullptr) { return; }

	for (int i = 0; i < m_lightList.size(); i++)
	{
		lightData[i].Setup(*m_lightList[i]);
	}

	memcpy(p, lightData.data(), totalSize);

	m_lightCB.Unmap(index);
}

DXUTILITY::Descriptor LightPool::GetDescriptor(int frameIndex)
{
	return m_lightCB.GetDescriptor(frameIndex);
}
