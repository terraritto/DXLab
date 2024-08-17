#include "RadialBlurPSO.h"
#include "../../../include/imgui/imgui.h"
#include "../../../include/imgui/imgui_impl_dx12.h"
#include "../../../include/imgui/imgui_impl_win32.h"

RadialBlurPSO::RadialBlurPSO(std::weak_ptr<DXUTILITY::Device> device)
	: PostPSO(device)
{
	m_constantBuffer.Initialize(m_device, GetCBSize() + 255 & ~255);
	m_constantBuffer.SetupBufferView(m_device);
	m_data = std::make_unique<RadialBlurData>();
}

void RadialBlurPSO::Update(double deltaTime)
{
	if (m_data == nullptr)
	{
		return;
	}

	ImGui::Begin("Radial Blur");

	ImGui::SliderFloat("SampleCount", &(m_data->m_sampleCount), 4.0f, 16.0f);
	ImGui::SliderFloat("Strength", &(m_data->m_strength), 0.0f, 1.0f);
	ImGui::SliderFloat2("UV", m_data->m_uv, -1.0f, 1.0f);

	ImGui::End();
}

void RadialBlurPSO::WriteCBData()
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
