#include "SimpleTriangleApp.h"
#include "../../../MyFramework/Utility/Win32App.h"
#include "../../../include/d3dx12.h"
#include "../../../MyFramework/SO/ShaderUtility.h"

SimpleTriangleApp::SimpleTriangleApp(UINT width, UINT height)
	: RenderInterface(width, height, L"SimpleTriangle")
{
}

void SimpleTriangleApp::Initialize()
{
	// DX12�̏�����
	if (InitializeGraphicsDevice(Win32Application::GetHWND()) == false)
	{
		throw std::runtime_error("Failed Initialize Device.");
	}

	// vertex/index��p��
	Vertex triangleVertices[] =
	{
		{{0.0f, 0.25f, 0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}},
		{{0.25f, -0.25f, 0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}},
		{{-0.25f, -0.25f, 0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}},
	};
	uint32_t indices[] = { 0,1,2 };
	m_indexCount = _countof(indices);

	// vertex/index�o�b�t�@�̐���
	{
		m_vertexBuffer = m_device->CreateBuffer(sizeof(triangleVertices), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD, nullptr);
		void* mapped;
		CD3DX12_RANGE range(0, 0);
		HRESULT hr = m_vertexBuffer->Map(0, &range, &mapped);
		if (SUCCEEDED(hr))
		{
			memcpy(mapped, triangleVertices, sizeof(triangleVertices));
		}
	}

	{
		m_indexBuffer = m_device->CreateBuffer(sizeof(indices), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD, nullptr);
		void* mapped;
		CD3DX12_RANGE range(0, 0);
		HRESULT hr = m_indexBuffer->Map(0, &range, &mapped);
		if (SUCCEEDED(hr))
		{
			memcpy(mapped, indices, sizeof(indices));
		}
	}

	// BufferView���쐬
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = sizeof(triangleVertices);
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);

	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = sizeof(indices);
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	// shader�̃R���p�C��
	auto vsByte = LoadShaderForRaster(L"SimpleTriangleVS.dxlib", m_shadervs);
	auto psByte = LoadShaderForRaster(L"SimpleTrianglePS.dxlib", m_shaderps);

	// RootSignature�̐���
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc{};
	rootSigDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	m_rootSignature = CreateRootSignature(rootSigDesc);

	// InputLayout
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDesc =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, Pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, Color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
	};

	// PSO�̐���
	m_psoContainer.SetVSByte(vsByte);
	m_psoContainer.SetPSByte(psByte);
	m_psoContainer.SetRootSignature(m_rootSignature);
	m_psoContainer.SetInputElement(inputElementDesc);
	m_psoContainer.CreatePSO(m_device->GetDevice());

	// CommandList
	m_commandList = m_device->CreateCommandList();
	m_commandList->Close();
}

void SimpleTriangleApp::Destroy()
{
}

void SimpleTriangleApp::Update()
{
}

void SimpleTriangleApp::OnRender()
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

	// PSO�̃Z�b�g
	m_commandList->SetPipelineState(m_psoContainer.GetPSO().Get());

	// RootSignature�̃Z�b�g
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	
	// viewport/scissor�̃Z�b�g
	m_commandList->RSSetViewports(1, &m_device->GetDefaultViewport());
	m_commandList->RSSetScissorRects(1, &m_device->GetDefaultScissorRect());

	// Premitive�̃Z�b�g
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->IASetIndexBuffer(&m_indexBufferView);

	// �R�}���h�̔��s
	m_commandList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);

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