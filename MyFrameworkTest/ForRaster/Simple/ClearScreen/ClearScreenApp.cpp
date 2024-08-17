#include "ClearScreenApp.h"
#include "../../../MyFramework/Utility/Win32App.h"
#include "../../../include/d3dx12.h"

ClearScreenApp::ClearScreenApp(UINT width, UINT height)
	: RenderInterface(width, height, L"ClearScreen")
{
}

void ClearScreenApp::Initialize()
{
	// DX12�̏�����
	if (InitializeGraphicsDevice(Win32Application::GetHWND()) == false)
	{
		throw std::runtime_error("Failed Initialize Device.");
	}

	// CommandList
	m_commandList = m_device->CreateCommandList();
	m_commandList->Close();
}

void ClearScreenApp::Destroy()
{
}

void ClearScreenApp::Update()
{
}

void ClearScreenApp::OnRender()
{
	auto device = m_device->GetDevice();
	auto frameIndex = m_device->GetCurrentFrameIndex();

	// Allocator�̃��Z�b�g
	auto allocator = m_device->GetCurrentCommandAllocator();
	allocator->Reset();

	// commandlist�̃��Z�b�g
	m_commandList->Reset(allocator.Get(), nullptr);

	auto renderTarget = m_device->GetRT();

	// swapchain�\���\����RTV�`��\��
	auto barrerToRT = CD3DX12_RESOURCE_BARRIER::Transition
	(
		renderTarget.Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	m_commandList->ResourceBarrier(1, &barrerToRT);

	// �J���[�o�b�t�@�̃N���A
	const float clearColor[] = { 0.1f, 0.25f, 0.5f, 0.0f };
	m_commandList->ClearRenderTargetView(
		m_device->GetRTV(), clearColor, 0, nullptr);

	// �f�v�X�o�b�t�@�̃N���A
	m_commandList->ClearDepthStencilView(
		m_device->GetDSV(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// �`�����Z�b�g
	m_commandList->OMSetRenderTargets(1, &m_device->GetRTV(), FALSE, &m_device->GetDSV());

	// RTV�`��\->swapchain�\���\
	auto barrierToPresent = CD3DX12_RESOURCE_BARRIER::Transition
	(
		renderTarget.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);
	m_commandList->ResourceBarrier(1, &barrierToPresent);

	m_commandList->Close();

	m_device->ExecuteCommandList(m_commandList);

	m_device->Present(1);
}
