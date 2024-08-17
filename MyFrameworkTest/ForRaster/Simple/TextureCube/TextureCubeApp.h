#pragma once

#pragma once

#include <memory>
#include "../../../MyFramework/Utility/RenderInterface.h"
#include "../../../MyFramework/Utility/TypeUtility.h"
#include "../../../MyFramework/PSO/PSOContainer.h"

class TextureCubeApp : public RenderInterface
{
public:
	TextureCubeApp(UINT width, UINT height);

public:
	virtual void Initialize() override;
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void OnRender() override;

	struct ShaderParameters
	{
		XMFLOAT4X4 mtxWorld;
		XMFLOAT4X4 mtxView;
		XMFLOAT4X4 mtxProj;
	};

protected:
	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_indexBuffer;
	DXUTILITY::TextureResource m_texture;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

	std::vector<char> m_shadervs, m_shaderps;

	UINT m_indexCount;

	ComPtr<ID3D12RootSignature> m_rootSignature;
	PSOContainer m_psoContainer;

	DXUTILITY::DynamicBuffer m_sceneCB;
	std::unique_ptr<DXUTILITY::SamplerDescriptor> m_sampler;

private:
	ComPtr<ID3D12GraphicsCommandList4> m_commandList;
};
