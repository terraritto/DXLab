#include "ResultBuffer.h"

void ResultBuffer::Initialize(DXUTILITY::Device* device, UINT width, UINT height, DXGI_FORMAT format, DirectX::XMVECTORF32 clearColor)
{
	// output—pTexture
	m_output = device->CreateTexture2D
	(
		width, height, format,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_HEAP_TYPE_DEFAULT, DXGI_SAMPLE_DESC{ 1, 0 },
		clearColor
	);

	m_depth = device->CreateTexture2D
	(
		width, height, DXGI_FORMAT_R32_TYPELESS,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		D3D12_HEAP_TYPE_DEFAULT, DXGI_SAMPLE_DESC{ 1, 0 },
		clearColor
	);

	// descriptor
	m_outputDescriptor = device->AllocateDescriptor();
	m_outputDescriptorRTV = device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_outputDescriptorDSV = device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	auto dxDevice = device->GetDevice();
	dxDevice->CreateRenderTargetView
	(
		m_output.Get(),
		&rtvDesc,
		m_outputDescriptorRTV.m_cpuHandle
	);

	dxDevice->CreateUnorderedAccessView
	(
		m_output.Get(),
		nullptr,
		&uavDesc,
		m_outputDescriptor.m_cpuHandle
	);

	dxDevice->CreateDepthStencilView
	(
		m_depth.Get(),
		&dsvDesc,
		m_outputDescriptorDSV.m_cpuHandle
	);

	m_outputDescriptorDepth = device->CreateSRV
	(
		m_depth.Get(),
		&srvDesc
	);
}
