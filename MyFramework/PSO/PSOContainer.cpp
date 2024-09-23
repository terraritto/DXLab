#include "PSOContainer.h"
#include "../SO/ShaderUtility.h"
#include <stdexcept>

PSOContainer::PSOContainer()
{
	// デフォルト値の生成
	m_psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	m_psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	m_psoDesc.NumRenderTargets = 1;
	m_psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	m_psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	m_psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	m_psoDesc.SampleDesc = { 1,0 };
	m_psoDesc.SampleMask = UINT_MAX;
	m_psoDesc.RasterizerState.DepthBias = 0.05f;
}

void PSOContainer::SetVSByte(D3D12_SHADER_BYTECODE& byte)
{
	m_psoDesc.VS = byte;
}

void PSOContainer::SetVSByte(std::wstring filePath)
{
	auto vsByte = LoadShaderForRaster(filePath, m_vs);
	m_psoDesc.VS = vsByte;

}

void PSOContainer::SetPSByte(D3D12_SHADER_BYTECODE& byte)
{
	m_psoDesc.PS = byte;
}

void PSOContainer::SetPSByte(std::wstring filePath)
{
	auto psByte = LoadShaderForRaster(filePath, m_ps);
	m_psoDesc.PS = psByte;
}

void PSOContainer::SetRTVFormat(int index, DXGI_FORMAT format)
{
	m_psoDesc.RTVFormats[index] = format;
}

void PSOContainer::SetCullMode(D3D12_CULL_MODE mode)
{
	m_psoDesc.RasterizerState.CullMode = mode;
}

void PSOContainer::IsDepthClip(bool isClip)
{
	m_psoDesc.RasterizerState.DepthClipEnable = isClip;
}

void PSOContainer::SetRootSignature(ComPtr<ID3D12RootSignature>& rs)
{
	m_psoDesc.pRootSignature = rs.Get();
}

void PSOContainer::SetInputElement(std::vector<D3D12_INPUT_ELEMENT_DESC>& ieList)
{
	m_inputElementList = ieList;
	m_psoDesc.InputLayout = { m_inputElementList.data(), static_cast<unsigned int>(ieList.size())};
}

void PSOContainer::SetBlendData(const BlendData& data)
{
	auto& target = m_psoDesc.BlendState.RenderTarget[0];
	target.BlendEnable = data.m_isBlend;
	target.SrcBlend = data.m_srcBlend;
	target.DestBlend = data.m_destBlend;
	target.SrcBlendAlpha = data.m_srcBlendAlpha;
	target.DestBlendAlpha = data.m_destBlendAlpha;
}

void PSOContainer::SetSampleDesc(const DXGI_SAMPLE_DESC desc)
{
	m_psoDesc.SampleDesc = desc;
}

void PSOContainer::CreatePSO(ComPtr<ID3D12Device> device)
{
	HRESULT hr = device->CreateGraphicsPipelineState(&m_psoDesc, IID_PPV_ARGS(m_pso.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		throw std::runtime_error("Create PSO failed");
	}
}
