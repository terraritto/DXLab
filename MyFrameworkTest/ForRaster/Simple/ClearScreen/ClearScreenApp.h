#pragma once

#include <memory>
#include "../../../MyFramework/Utility/RenderInterface.h"

class ClearScreenApp : public RenderInterface
{
public:
	ClearScreenApp(UINT width, UINT height);

public:
	virtual void Initialize() override;
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void OnRender() override;

private:
	ComPtr<ID3D12GraphicsCommandList4> m_commandList;
};
