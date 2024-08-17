#pragma once
#include <d3d12.h>
#include "../../Utility/TypeUtility.h"
#include "../../Utility/DXUtility.h"

class LocalRS
{
public:
	virtual void Initialize(DXUTILITY::Device* device) = 0;

	ID3D12RootSignature* GetLocalRS() const { return m_localRS.Get(); }

protected:
	ComPtr<ID3D12RootSignature> m_localRS;
};