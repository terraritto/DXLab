#pragma once
#define NOMINMAX
#include <memory>
#include <numeric>
#include "..//../MyFramework/Utility/RenderInterface.h"
#include "../../MyFramework/AS/BLAS/BLASManager.h"
#include "../../MyFramework/Sampler/Sampler.h"
#include "../../MyFramework/Result/ResultBuffer.h"
#include "../../MyFramework/Geometry/Loader/DefaultMeshLoader.h"
#include "../../MyFramework/Utility/Camera.h"
#include "../../MyFramework/RS/CommonGRS.h"
#include "../../MyFramework/PSO/PostPSO.h"
#include "./AabbLRS.h"
#include "./PathTraceSO.h"
#include "./PathTraceShaderTable.h"
#include "./PathTraceRGLRS.h"
#include "./DefaultMeshLRS.h"

class CommonGRS;

class PathTraceRenderer : public RenderInterface
{
public:
	PathTraceRenderer(UINT width, UINT height);

public:
	virtual void Initialize() override;
	virtual void Destroy() override;
	virtual void Update(double DeltaTime) override;
	virtual void OnRender() override;

protected:
	void InitializePostProcess();
	void RenderToTexture();

private:
	std::unique_ptr<BLASManager> m_blasProcessor;
	std::unique_ptr<CommonGRS> m_grsProcessor;
	std::unique_ptr<PathTraceSO> m_soProcessor;
	std::unique_ptr<ResultBuffer> m_resultBuffer;
	std::unique_ptr<PathTraceShaderTable> m_shaderTable;

	std::unique_ptr<DefaultMeshLoader> m_meshLoader;
	std::wstring m_sponzaFileName, m_moriFileName;

	// loot signature
	std::unique_ptr<PathTraceRGLRS> m_rsRGS;		// RayGen�V�F�[�_�[�̃��[�J�����[�g�V�O�l�`��
	std::unique_ptr<AabbLRS> m_rsSphere;			// AABB�̃��[�J�����[�g�V�O�l�`��(sphere)
	std::unique_ptr<DefaultMeshLRS> m_rsMesh;		// Mesh�̃��[�J�����[�g�V�O�l�`��

	// post
	std::vector<std::shared_ptr<PostPSO>> m_postPSOList;
	ComPtr<ID3D12RootSignature> m_postRootSignature;

private:
	// scene�p
	struct SceneParam
	{
		XMMATRIX mtxView;       // �r���[�s��.
		XMMATRIX mtxProj;       // �v���W�F�N�V�����s��.
		XMMATRIX mtxViewInv;    // �r���[�t�s��.
		XMMATRIX mtxProjInv;    // �v���W�F�N�V�����t�s��.
		XMVECTOR ambientColor;  // ����.
		XMVECTOR eyePosition;	// ���_.
		uint32_t lightCount;	// ���C�g�̍ő吔
		uint32_t loopCount;		// ������
	};
	SceneParam m_sceneParam;
	DXUTILITY::Camera m_camera; // �J����

	bool m_isClear; // RenderTarget���N���A���邩�H
	static constexpr uint32_t MAX_LOOP_COUNT = 8192;

private:
	// global buffer�֌W
	DXUTILITY::DynamicConstantBuffer m_sceneCB;
	std::unique_ptr<Sampler> m_sampler;
	double m_samplerTimer;

	// skybox
	DXUTILITY::TextureResource m_hdrBackground;

private:
	ComPtr<ID3D12GraphicsCommandList4> m_commandList;
};
