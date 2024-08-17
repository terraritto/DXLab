#pragma once
#include "../Utility/DXUtility.h"
#include "../Utility/TypeUtility.h"

class ResultBuffer
{
public:
	virtual void Initialize(DXUTILITY::Device* device, UINT width, UINT height, 
		DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT, DirectX::XMVECTORF32 clearColor = DirectX::g_XMZero);

	ComPtr<ID3D12Resource> GetOutputBuffer() const { return m_output; }
	DXUTILITY::Descriptor* GetOutputDescriptor() { return &m_outputDescriptor; }
	DXUTILITY::Descriptor* GetOutputRTVDescriptor() { return &m_outputDescriptorRTV; }

	ComPtr<ID3D12Resource> GetDepthBuffer() const { return m_depth; }
	DXUTILITY::Descriptor* GetOutputDepthDescriptor() { return &m_outputDescriptorDepth; }
	DXUTILITY::Descriptor* GetOutputDSVDescriptor() { return &m_outputDescriptorDSV; }

protected:
	ComPtr<ID3D12Resource> m_output, m_depth;
	DXUTILITY::Descriptor m_outputDescriptor, m_outputDescriptorDepth;
	DXUTILITY::Descriptor m_outputDescriptorRTV, m_outputDescriptorDSV;
};