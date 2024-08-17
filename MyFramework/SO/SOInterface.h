#pragma once
#include "SOContainer.h"
#include "../Utility/DXUtility.h"

class SOInterface
{
public:
	virtual void Initialize(DXUTILITY::Device* device, ComPtr<ID3D12RootSignature> rsResource) {};

	// lrs—p
	virtual void Initialize(DXUTILITY::Device* device, ComPtr<ID3D12RootSignature> grsResource, std::vector<ID3D12RootSignature*> lrsResourceList) {}

	ComPtr<ID3D12StateObject> GetRtState() { return m_rtState; }

protected:
	SOContainer m_container;

	ComPtr<ID3D12StateObject> m_rtState;
};