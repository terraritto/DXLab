#pragma once
#include "MaterialBase.h"

class DefaultMeshMaterial : public MaterialBase
{
public:
	DefaultMeshMaterial();

	// 各種設定
	void SetAlbedo(const XMVECTOR& albedo) { m_albedo = albedo; }
	const XMVECTOR GetAlbedo() const { return m_albedo; }

	void SetSpecular(const XMVECTOR& specular) { m_specular = specular; }
	const XMVECTOR GetSpecular() const { return m_specular; }

	void SetSpecularPow(const float& specular) { m_specularPow = specular; }
	const float GetSpecularPow() const { return m_specularPow; }

	void SetAlpha(const float& alpha) { m_alpha = alpha; if (m_alpha != 1.0f) { m_isAlpha = true; } }
	const float GetAlpha() const { return m_alpha; }
	const bool GetIsAlpha()const { return m_isAlpha; }

	void SetColorTexture(const std::wstring filePath);
	std::weak_ptr<DXUTILITY::TextureResource> GetColorTexture();
	std::wstring GetColorTextureName() const { return this->m_texturePath[0]; }

	void SetDiffuseTexture(const std::wstring filePath);
	std::weak_ptr<DXUTILITY::TextureResource> GetDiffuseTexture();
	std::wstring GetDiffuseTextureName() const { return this->m_texturePath[1]; }

	void SetNormalTexture(const std::wstring filePath);
	std::weak_ptr<DXUTILITY::TextureResource> GetNormalTexture();
	std::wstring GetNormalTextureName() const { return this->m_texturePath[2]; }

	virtual void WriteData(uint8_t* mapped) override
	{
		m_parameter.param1.m128_f32[3] = m_alpha;
		m_parameter.param2 = m_albedo; 	m_parameter.param2.m128_f32[3] = 1.0;
		m_parameter.param3 = m_specular; m_parameter.param3.m128_f32[3] = m_specularPow;
		m_parameter.index = 0;

		int TotalSize = GetTotalBufferSize();

		memcpy(mapped, &m_parameter, TotalSize);
		mapped += TotalSize;
	}

	// 総メモリサイズ
	virtual const int GetTotalBufferSize() const override { return sizeof(MaterialParameter); }

protected:
	struct MaterialParameter
	{
		XMVECTOR param1, param2, param3, param4;
		int index;
	};

protected:
	// 必要パラメータ
	XMVECTOR m_albedo;
	XMVECTOR m_specular;
	float m_specularPow;
	float m_alpha;
	bool m_isAlpha;

	// Texture
	std::vector<std::wstring> m_texturePath;
	//std::wstring m_colorTexPath;
	//std::wstring m_metallicRoughnessPath;
	//std::wstring m_normalPath;

	MaterialParameter m_parameter;
};