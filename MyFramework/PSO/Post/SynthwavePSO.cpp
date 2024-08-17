#include "SynthwavePSO.h"
#include "../../../include/imgui/imgui.h"
#include "../../../include/imgui/imgui_impl_dx12.h"
#include "../../../include/imgui/imgui_impl_win32.h"

SynthwavePSO::SynthwavePSO(std::weak_ptr<DXUTILITY::Device> device)
	: PostPSO(device)
{
	m_constantBuffer.Initialize(m_device, GetCBSize() + 255 & ~255);
	m_constantBuffer.SetupBufferView(m_device);
	m_data = std::make_unique<SynthwaveData>();
}

void SynthwavePSO::Update(double deltaTime)
{
	if (m_data == nullptr)
	{
		return;
	}

	ImGui::Begin("Synthwave");

	ImGui::SliderFloat("Star Threshold", &m_data->m_backColorLower[3], 0.0f, 1.0f);

	ImGui::SliderFloat3("BackColor Upper", m_data->m_backColorUpper, 0.0f, 1.0f);
	ImGui::SliderFloat4("BackColor Lower", m_data->m_backColorLower, 0.0f, 1.0f);
	ImGui::SliderFloat("Back Threshold", &m_data->m_backColorUpper[3], 0.0f, 1.0f);

	ImGui::SliderFloat2("UVOffset", m_data->m_uvTime, 0.0f, 1.0f);
	ImGui::SliderFloat3("SunColor", m_data->m_sunColor, 0.0f, 1.0f);
	ImGui::SliderFloat("SphereSize", &m_data->m_uvTime[2], 0.0f, 0.5f);

	ImGui::End();
}

void SynthwavePSO::WriteCBData()
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
