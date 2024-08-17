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
	// DX12の初期化
	if (InitializeGraphicsDevice(Win32Application::GetHWND()) == false)
	{
		throw std::runtime_error("Failed Initialize Device.");
	}

	// vertex/indexを用意
	Vertex triangleVertices[] =
	{
		{{0.0f, 0.25f, 0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}},
		{{0.25f, -0.25f, 0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}},
		{{-0.25f, -0.25f, 0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}},
	};
	uint32_t indices[] = { 0,1,2 };
	m_indexCount = _countof(indices);

	// vertex/indexバッファの生成
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

	// BufferViewを作成
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = sizeof(triangleVertices);
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);

	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = sizeof(indices);
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	// shaderのコンパイル
	auto vsByte = LoadShaderForRaster(L"SimpleTriangleVS.dxlib", m_shadervs);
	auto psByte = LoadShaderForRaster(L"SimpleTrianglePS.dxlib", m_shaderps);

	// RootSignatureの生成
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc{};
	rootSigDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	m_rootSignature = CreateRootSignature(rootSigDesc);

	// InputLayout
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDesc =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, Pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, Color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
	};

	// PSOの生成
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

	// Allocatorのリセット
	auto allocator = m_device->GetCurrentCommandAllocator();
	allocator->Reset();

	// commandlistのリセット
	m_commandList->Reset(allocator.Get(), nullptr);

	auto renderTarget = m_device->GetRT();

	// swapchain表示可能からRTV描画可能へ
	auto barrerToRT = CD3DX12_RESOURCE_BARRIER::Transition
	(
		renderTarget.Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	m_commandList->ResourceBarrier(1, &barrerToRT);

	// カラーバッファのクリア
	const float clearColor[] = { 0.1f, 0.25f, 0.5f, 0.0f };
	m_commandList->ClearRenderTargetView(
		m_device->GetRTV(), clearColor, 0, nullptr);

	// デプスバッファのクリア
	m_commandList->ClearDepthStencilView(
		m_device->GetDSV(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// 描画先をセット
	m_commandList->OMSetRenderTargets(1, &m_device->GetRTV(), FALSE, &m_device->GetDSV());

	// PSOのセット
	m_commandList->SetPipelineState(m_psoContainer.GetPSO().Get());

	// RootSignatureのセット
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	
	// viewport/scissorのセット
	m_commandList->RSSetViewports(1, &m_device->GetDefaultViewport());
	m_commandList->RSSetScissorRects(1, &m_device->GetDefaultScissorRect());

	// Premitiveのセット
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->IASetIndexBuffer(&m_indexBufferView);

	// コマンドの発行
	m_commandList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);

	// RTV描画可能->swapchain表示可能
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