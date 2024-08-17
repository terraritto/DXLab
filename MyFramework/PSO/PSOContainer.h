#pragma once
#include "../../include/d3dx12.h"
#include "../../MyFramework/Utility/TypeUtility.h"
#include <vector>

struct BlendData
{
	bool m_isBlend;
	D3D12_BLEND m_srcBlend, m_destBlend;
	D3D12_BLEND m_srcBlendAlpha, m_destBlendAlpha;

	BlendData(bool isBlend, 
		D3D12_BLEND srcBlend, D3D12_BLEND destBlend,
		D3D12_BLEND srcBlendAlpha, D3D12_BLEND destBlendalpha)
		: m_isBlend(isBlend)
		, m_srcBlend(srcBlend)
		, m_destBlend(destBlend)
		, m_srcBlendAlpha(srcBlendAlpha)
		, m_destBlendAlpha(destBlendalpha)
	{}
};

class PSOContainer
{
public:
	PSOContainer();

	// VS/PS�̐ݒ�
	void SetVSByte(D3D12_SHADER_BYTECODE& byte);
	void SetVSByte(std::wstring filePath);
	void SetPSByte(D3D12_SHADER_BYTECODE& byte);
	void SetPSByte(std::wstring filePath);

	// Culling�̃��[�h�w��
	void SetCullMode(D3D12_CULL_MODE mode);

	// DepthClip�̐ݒ�
	void IsDepthClip(bool isClip);

	// RootSignature�̐ݒ�
	void SetRootSignature(ComPtr<ID3D12RootSignature>& rs);

	// InputElement�̐ݒ�
	void SetInputElement(std::vector<D3D12_INPUT_ELEMENT_DESC>& ieList);

	// Blend�f�[�^���w��
	void SetBlendData(const BlendData& data);

	// Sample�̐ݒ�
	void SetSampleDesc(const DXGI_SAMPLE_DESC desc);

	// PSO�̐���
	void CreatePSO(ComPtr<ID3D12Device> device);

	// PSO�̎擾
	ComPtr<ID3D12PipelineState> GetPSO() const { return m_pso; }

protected:
	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_psoDesc;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputElementList;

	ComPtr<ID3D12PipelineState> m_pso;

	std::vector<char> m_vs, m_ps;
};