#include "FXAAPSO.h"
#include "../../../include/imgui/imgui.h"
#include "../../../include/imgui/imgui_impl_dx12.h"
#include "../../../include/imgui/imgui_impl_win32.h"

FXAAPSO::FXAAPSO(std::weak_ptr<DXUTILITY::Device> device)
	: PostPSO(device)
{
	m_constantBuffer.Initialize(m_device, GetCBSize() + 255 & ~255);
	m_constantBuffer.SetupBufferView(m_device);
	m_data = std::make_unique<FXAAData>();
}

void FXAAPSO::Update(double deltaTime)
{
	if (m_data == nullptr)
	{
		return;
	}

	ImGui::Begin("FXAA");
	ImGui::SliderFloat("UseFXAA", &(m_data->m_useFXAA), 0.0f, 1.0f);
	ImGui::End();

	if (auto pDevice = m_device.lock())
	{
		m_data->m_inv[0] = 1.0f / pDevice->GetScreenWidth();
		m_data->m_inv[1] = 1.0f / pDevice->GetScreenHeight();
	}
}

void FXAAPSO::WriteCBData()
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
