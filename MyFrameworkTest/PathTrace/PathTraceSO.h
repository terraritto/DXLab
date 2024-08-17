#pragma once
#include "../../MyFramework/SO/SOInterface.h"

class PathTraceSO : public SOInterface
{
public:
	virtual void Initialize(DXUTILITY::Device* device, ComPtr<ID3D12RootSignature> grsResource, std::vector<ID3D12RootSignature*> lrsResourceList) override;
};
