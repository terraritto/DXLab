#include "PostPSO.h"
#include "../../../include/d3dx12.h"
#include "../Geometry/GeometryDefinitions.h"

PostPSO::PostPSO(std::weak_ptr<DXUTILITY::Device> device)
	: PSOContainer()
{
	m_device = device;
	auto pDevice = m_device.lock();

	// vertex/indexを用意
	VertexPT triangleVertices[] =
	{
		{{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
		{{ 1.0f,  1.0f, 0.0f}, {1.0f, 0.0f}},
		{{ 1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
		{{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f}},
	};
	uint32_t indices[] = { 0,1,2,3,1,0 };
	m_indexCount = _countof(indices);

	// vertex/indexバッファの生成
	{
		m_vertexBuffer = pDevice->CreateBuffer(sizeof(triangleVertices), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD, nullptr);
		void* mapped;
		CD3DX12_RANGE range(0, 0);
		HRESULT hr = m_vertexBuffer->Map(0, &range, &mapped);
		if (SUCCEEDED(hr))
		{
			memcpy(mapped, triangleVertices, sizeof(triangleVertices));
		}
	}

	{
		m_indexBuffer = pDevice->CreateBuffer(sizeof(indices), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD, nullptr);
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
	m_vertexBufferView.StrideInBytes = sizeof(VertexPT);

	
	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = sizeof(indices);
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	// InputLayout
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDesc =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPT, Position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexPT, UV), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
	};
	SetInputElement(inputElementDesc);
}

PostPSO::~PostPSO()
{
	auto pDevice = m_device.lock();

	if (pDevice)
	{
		m_constantBuffer.UnInitialize(pDevice);
	}
}

void PostPSO::Draw(ComPtr<ID3D12GraphicsCommandList4>& commandList, D3D12_GPU_DESCRIPTOR_HANDLE& handle)
{
	int index = 0;
	if (auto pDevice = m_device.lock())
	{
		index = pDevice->GetCurrentFrameIndex();
	}

	// 更新を行う
	WriteCBData();

	// PSOのセット
	commandList->SetPipelineState(m_pso.Get());

	// Primitiveのセット
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->IASetIndexBuffer(&m_indexBufferView);

	commandList->SetGraphicsRootDescriptorTable(0, handle);
	if (m_constantBuffer.IsInitialize())
	{
		commandList->SetGraphicsRootDescriptorTable(1, m_constantBuffer.GetDescriptor(index).m_gpuHandle);
	}
	// コマンドの発行
	commandList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}
