#pragma once

#include <memory>
#include "../../../MyFramework/Geometry/Loader/DefaultMeshLoader.h"
#include "../../../MyFramework/Utility/RenderInterface.h"
#include "../../../MyFramework/Utility/TypeUtility.h"
#include "../../../MyFramework/Utility/Camera.h"
#include "../../../MyFramework/PSO/PSOContainer.h"
#include "../../../MyFramework/PSO/PostPSO.h"
#include "../../../MyFramework/Result/ResultBuffer.h"

class SimpleModelApp : public RenderInterface
{
public:
	SimpleModelApp(UINT width, UINT height);

public:
	virtual void Initialize() override;
	virtual void Destroy() override;
	virtual void Update(double DeltaTime) override;
	virtual void OnRender() override;

	struct ShaderParameters
	{
		XMFLOAT4X4 mtxWorld;
		XMFLOAT4X4 mtxView;
		XMFLOAT4X4 mtxProj;
		XMVECTOR cameraParam;
		int lightIndex;
	};

	struct SceneShadowParameters
	{
		XMFLOAT4X4 LVP;
		float offset = 0.1f;
	};

	struct ShadowParameters
	{
		XMFLOAT4X4 mtxWorld;
		XMFLOAT4X4 mtxView;
		XMFLOAT4X4 mtxProj;
	};

protected:
	std::unique_ptr<DefaultMeshLoader> m_meshLoader;
	std::wstring m_fileName, m_modelName;

	DXUTILITY::Camera m_camera; // �J����
	
	ComPtr<ID3D12RootSignature> m_rootSignature;
	PSOContainer m_psoContainer;
	PSOContainer m_AlphaPsoContainer;
	PSOContainer m_rimContainer;

	DXUTILITY::DynamicBuffer m_sceneCB, m_sceneShadowCB;
	std::unique_ptr<DXUTILITY::SamplerDescriptor> m_sampler;

protected:
	// Shadow�p
	DXUTILITY::Camera m_shadowCamera; // ShadowMap�p�J����
	DXUTILITY::DynamicBuffer m_shadowCB;
	PSOContainer m_shadowMapPSO;
	std::unique_ptr<ResultBuffer> m_shadowMap;
	ComPtr<ID3D12RootSignature> m_shadowMapRS;

protected:
	// �p��
	void SetupMesh();
	void SetupBuffer();
	void SetupShadowMap();
	XMFLOAT3 GetDirection();

	// Postprocess�n
	void RenderToShadow();
	void RenderToTexture();
	void RenderToMSAA();
	void RenderToMain();

	std::vector<std::unique_ptr<PostPSO>> m_postPSOList;
	std::unique_ptr<ResultBuffer> m_result, m_msaa;
	ComPtr<ID3D12RootSignature> m_postRootSignature;

private:
	ComPtr<ID3D12GraphicsCommandList4> m_commandList;
};
