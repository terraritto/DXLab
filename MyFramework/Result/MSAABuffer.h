#pragma once
#include "ResultBuffer.h"

class MSAABuffer : public ResultBuffer
{
public:
	void Initialize(DXUTILITY::Device* device, UINT width, UINT height, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT, DirectX::XMVECTORF32 clearColor = DirectX::g_XMZero) override;
};
