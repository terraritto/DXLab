#pragma once
#include <d3d12.h>
#include "../../Utility/TypeUtility.h"
#include "../../Utility/DXUtility.h"

class GlobalRS
{
public:
	virtual void Initialize(DXUTILITY::Device* device) = 0;

	ComPtr<ID3D12RootSignature> GetGlobalRS() const { return m_globalRS; }

protected:
	ComPtr<ID3D12RootSignature> m_globalRS;
};