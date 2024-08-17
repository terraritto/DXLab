#pragma once

#include <memory>
#include "../../../MyFramework/Utility/RenderInterface.h"
#include "../../../MyFramework/Utility/TypeUtility.h"
#include "../../../MyFramework/PSO/PSOContainer.h"
#include "../../../MyFramework/Utility/FileUtility.h"

class ShaderToyTest : public RenderInterface
{
public:
	ShaderToyTest(UINT width, UINT height);

public:
	virtual void Initialize() override;
	virtual void Destroy() override;
	virtual void Update(double DeltaTime) override;
	virtual void OnRender() override;

protected:
	// ’¸“_
	struct Vertex
	{
		XMFLOAT4 Pos;
		XMFLOAT2 UV;
	};

protected:
	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_indexBuffer;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

	UINT m_indexCount;

	DiffNotifier m_notifier;
	double m_timer;

	ComPtr<ID3D12RootSignature> m_rootSignature;
	PSOContainer m_psoContainerSrc, m_psoContainerDest;
	bool m_isUseDest;

private:
	ComPtr<ID3D12GraphicsCommandList4> m_commandList;

	const std::wstring parentPath = L"MyFrameworkTest/ForRaster/Simple/ShaderToyTest/";
	const std::wstring vertexPath = parentPath + L"ShaderToyVS.hlsl";
	const std::wstring pixelPath = parentPath + L"ShaderToyPS.hlsl";
};
