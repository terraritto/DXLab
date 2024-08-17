#include "MSAABuffer.h"

void MSAABuffer::Initialize(DXUTILITY::Device* device, UINT width, UINT height, DXGI_FORMAT format, DirectX::XMVECTORF32 clearColor)
{
	// Sampler‚ð•ªŠò
	DXGI_SAMPLE_DESC sampler;
	sampler.Quality = 0;
	sampler.Count = device->m_global->msaaSample;

	// output—pTexture
	m_output = device->CreateTexture2D
	(
		width, height, format,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
		D3D12_HEAP_TYPE_DEFAULT, sampler, clearColor
	);

	m_depth = device->CreateTexture2D
	(
		width, height, DXGI_FORMAT_D32_FLOAT,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		D3D12_HEAP_TYPE_DEFAULT, sampler, clearColor
	);

	// descriptor
	m_outputDescriptor = device->AllocateDescriptor();
	m_outputDescriptorRTV = device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_outputDescriptorDSV = device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	auto dxDevice = device->GetDevice();

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;

	dxDevice->CreateRenderTargetView
	(
		m_output.Get(),
		&rtvDesc,
		m_outputDescriptorRTV.m_cpuHandle
	);

	dxDevice->CreateDepthStencilView
	(
		m_depth.Get(),
		&dsvDesc,
		m_outputDescriptorDSV.m_cpuHandle
	);
}
