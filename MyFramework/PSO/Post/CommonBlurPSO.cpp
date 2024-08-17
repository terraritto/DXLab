#include "CommonBlurPSO.h"
#include "../../../include/imgui/imgui.h"
#include "../../../include/imgui/imgui_impl_dx12.h"
#include "../../../include/imgui/imgui_impl_win32.h"

CommonBlurPSO::CommonBlurPSO(std::weak_ptr<DXUTILITY::Device> device)
	: PostPSO(device)
{
	m_constantBuffer.Initialize(m_device, GetCBSize() + 255 & ~255);
	m_constantBuffer.SetupBufferView(m_device);
	m_data = std::make_unique<CommonBlurData>();

	// texelsize‚¾‚¯‚Í“ü‚ê‚Æ‚­
	auto pDevice = device.lock();
	m_data->m_texelSize[0] = 1.0f / pDevice->GetDefaultViewport().Width;
	m_data->m_texelSize[1] = 1.0f / pDevice->GetDefaultViewport().Height;
}

void CommonBlurPSO::Update(double deltaTime)
{
	ImGui::Begin("CommonBlur");

	XMFLOAT3 camPos;
	ImGui::SliderInt("Blur Method", &(m_data->m_index), 0, 3);
	ImGui::SliderFloat("sigma", &(m_data->m_sigma), 1.0f, 10.0f);
	ImGui::End();
}

void CommonBlurPSO::WriteCBData()
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
