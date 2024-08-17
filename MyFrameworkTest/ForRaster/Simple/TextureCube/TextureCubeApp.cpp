#include "TextureCubeApp.h"
#include "../../../MyFramework/Geometry/GeometryDefinitions.h"
#include "../../../MyFramework/SO/ShaderUtility.h"
#include "../../../MyFramework/Utility/Win32App.h"
#include "../../../include/d3dx12.h"

TextureCubeApp::TextureCubeApp(UINT width, UINT height)
	: RenderInterface(width, height, L"SimpleTriangle")
{
}

void TextureCubeApp::Initialize()
{
	// DX12�̏�����
	if (InitializeGraphicsDevice(Win32Application::GetHWND()) == false)
	{
		throw std::runtime_error("Failed Initialize Device.");
	}

	// vertex/index��p��
	std::vector<VertexPCT> vertices;
	std::vector<UINT> indices;
	GetColoredCube(vertices, indices);

	int vertexSize = sizeof(VertexPCT) * vertices.size();
	int indexSize = sizeof(UINT) * indices.size();

	m_indexCount = indices.size();

	// vertex/index�o�b�t�@�̐���
	{
		m_vertexBuffer = m_device->CreateBuffer(vertexSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD, nullptr);
		void* mapped;
		CD3DX12_RANGE range(0, 0);
		HRESULT hr = m_vertexBuffer->Map(0, &range, &mapped);
		if (SUCCEEDED(hr))
		{
			memcpy(mapped, vertices.data(), vertexSize);
		}
	}

	{
		m_indexBuffer = m_device->CreateBuffer(indexSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD, nullptr);
		void* mapped;
		CD3DX12_RANGE range(0, 0);
		HRESULT hr = m_indexBuffer->Map(0, &range, &mapped);
		if (SUCCEEDED(hr))
		{
			memcpy(mapped, indices.data(), indexSize);
		}
	}

	// BufferView���쐬
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = vertexSize;
	m_vertexBufferView.StrideInBytes = sizeof(VertexPCT);

	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = indexSize;
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	// shader�̃R���p�C��
	auto vsByte = LoadShaderForRaster(L"TextureCubeVS.dxlib", m_shadervs);
	auto psByte = LoadShaderForRaster(L"TextureCubePS.dxlib", m_shaderps);

	// DescriptorRange��RootParam�𐶐�
	CD3DX12_DESCRIPTOR_RANGE cbv, srv, sampler;
	cbv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	srv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	sampler.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

	CD3DX12_ROOT_PARAMETER rootParams[3];
	rootParams[0].InitAsDescriptorTable(1, &cbv, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParams[1].InitAsDescriptorTable(1, &srv, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParams[2].InitAsDescriptorTable(1, &sampler, D3D12_SHADER_VISIBILITY_PIXEL);

	// RootSignature�̐���
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc{};
	rootSigDesc.Init(
		_countof(rootParams), rootParams, // pParameters
		0, nullptr, // pStaticSamplers
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	m_rootSignature = CreateRootSignature(rootSigDesc);

	// InputLayout
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDesc =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPCT, Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VertexPCT, Color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPCT, UV), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
	};

	// PSO�̐���
	m_psoContainer.SetVSByte(vsByte);
	m_psoContainer.SetPSByte(psByte);
	m_psoContainer.SetRootSignature(m_rootSignature);
	m_psoContainer.SetInputElement(inputElementDesc);
	m_psoContainer.CreatePSO(m_device->GetDevice());

	// �萔�o�b�t�@�֌W�̗p��
	m_sceneCB.Initialize(m_device.get(), sizeof(ShaderParameters) + 255 & ~255);
	m_sceneCB.SetupBufferView(m_device.get());

	// Texture�̗p��
	m_texture = DXUTILITY::LoadTextureFromFile(m_device.get(), L"texture.png");

	// Sampler�̗p��
	m_sampler = std::make_unique<DXUTILITY::SamplerDescriptor>();
	m_sampler->Initialize(m_device.get());

	// CommandList
	m_commandList = m_device->CreateCommandList();
	m_commandList->Close();
}

void TextureCubeApp::Destroy()
{
	m_sampler->UnInitialize(m_device.get());
	m_sceneCB.UnInitialize(m_device.get());
}

void TextureCubeApp::Update()
{
}

void TextureCubeApp::OnRender()
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

	// �e�s����Z�b�g���Ă���
	ShaderParameters shaderParams;
	DirectX::XMStoreFloat4x4(&shaderParams.mtxWorld, DirectX::XMMatrixRotationAxis(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), DirectX::XMConvertToRadians(45.0f)));
	auto mtxView = DirectX::XMMatrixLookAtLH
	(
		DirectX::XMVectorSet(0.0f, 3.0f, -5.0f, 0.0f),
		DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
	);
	auto mtxProj = DirectX::XMMatrixPerspectiveFovLH
	(
		DirectX::XMConvertToRadians(45.0f),
		m_device->GetDefaultViewport().Width / m_device->GetDefaultViewport().Height,
		0.1f, 100.0f
	);
	DirectX::XMStoreFloat4x4(&shaderParams.mtxView, DirectX::XMMatrixTranspose(mtxView));
	DirectX::XMStoreFloat4x4(&shaderParams.mtxProj, DirectX::XMMatrixTranspose(mtxProj));

	// �萔�o�b�t�@�̍X�V
	{
		void* p = m_sceneCB.Map(frameIndex);
		memcpy(p, &shaderParams, sizeof(ShaderParameters));
		m_sceneCB.Unmap(frameIndex);
	}

	// PSO�̃Z�b�g
	m_commandList->SetPipelineState(m_psoContainer.GetPSO().Get());

	// RootSignature�̃Z�b�g
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	// viewport/scissor�̃Z�b�g
	m_commandList->RSSetViewports(1, &m_device->GetDefaultViewport());
	m_commandList->RSSetScissorRects(1, &m_device->GetDefaultScissorRect());

	// DescriptorHeap�̃Z�b�g
	ID3D12DescriptorHeap* heaps[] =
	{
		m_device->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).Get(),
		m_device->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER).Get()
	};
	m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);

	// Primitive�̃Z�b�g
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->IASetIndexBuffer(&m_indexBufferView);

	// RootDescriptor�̃Z�b�g
	m_commandList->SetGraphicsRootDescriptorTable(0, m_sceneCB.GetDescriptor(frameIndex).m_gpuHandle);
	m_commandList->SetGraphicsRootDescriptorTable(1, m_texture.srv.m_gpuHandle);
	m_commandList->SetGraphicsRootDescriptorTable(2, m_sampler->GetDescriptor().m_gpuHandle);

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