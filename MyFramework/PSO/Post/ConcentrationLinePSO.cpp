#include "ConcentrationLinePSO.h"
#include "../../../include/imgui/imgui.h"
#include "../../../include/imgui/imgui_impl_dx12.h"
#include "../../../include/imgui/imgui_impl_win32.h"
#include "../../MyFramework/Utility/TimeManager.h"

ConcentrationLinePSO::ConcentrationLinePSO(std::weak_ptr<DXUTILITY::Device> device)
	: PostPSO(device)
{
	m_constantBuffer.Initialize(m_device, GetCBSize() + 255 & ~255);
	m_constantBuffer.SetupBufferView(m_device);
	m_data = std::make_unique<ConcentrateData>();
}

void ConcentrationLinePSO::Update(double deltaTime)
{
	if (m_data == nullptr)
	{
		return;
	}

	m_data->m_time += deltaTime * 0.001f;

	ImGui::Begin("Concentration Line");

	XMFLOAT3 camPos;
	ImGui::Text("Time: %.2f", m_data->m_time);
	ImGui::SliderFloat("Speed", &(m_data->m_speed), 0.0f, 1.0f);
	ImGui::SliderFloat("Noise Scale", &(m_data->m_noiseScale), 0.0f, 60.0f);
	ImGui::SliderFloat("Line Region", &(m_data->m_lineRegion), -1.0f, 1.0f);
	ImGui::SliderFloat2("UV", m_data->m_uv, -1.0f, 1.0f);
	ImGui::SliderFloat3("Color", m_data->m_color, 0.0f, 1.0f);

	ImGui::End();
}

void ConcentrationLinePSO::WriteCBData()
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
