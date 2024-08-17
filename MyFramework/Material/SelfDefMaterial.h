#pragma once
#include "MaterialBase.h"

class SelfDefMaterial : public MaterialBase
{
public:
	SelfDefMaterial() : 
		MaterialBase(),
		m_ambient(DirectX::XMVectorSet(0.5f, 0.5f, 0.5f, 1.0f)),
		m_albedo(DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f)),
		m_specular(DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f)),
		m_reflectColor(DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f)),
		m_albedoScale(1.0f), m_specularPow(1.0f),
		m_etaIn(1.0f), m_etaOut(1.0f),
		m_materialIndex(0)
	{
	}

	// 各種設定
	void SetAmbient(const XMVECTOR& ambient) { m_ambient = ambient; }
	const XMVECTOR GetAmbient()const { return m_ambient; }

	void SetAlbedo(const XMVECTOR& albedo) { m_albedo = albedo; }
	const XMVECTOR GetAlbedo() const { return m_albedo; }

	void SetAlbedoScale(const float& albedo) { m_albedoScale = albedo; }
	const float GetAlbedoScale() const { return m_albedoScale; }

	void SetSpecular(const XMVECTOR& specular) { m_specular = specular; }
	const XMVECTOR GetSpecular() const { return m_specular; }

	void SetSpecularPow(const float& specular) { m_specularPow = specular; }
	const float GetSpecularPow() const { return m_specularPow; }

	void SetReflectColor(const XMVECTOR& refColor) { m_reflectColor = refColor; }
	const XMVECTOR GetReflectColor() const { return m_reflectColor; }

	void SetEtaIn(const float etaIn) { m_etaIn = etaIn; }
	const float GetEtaIn() const { return m_etaIn; }

	void SetEtaOut(const float etaOut) { m_etaOut = etaOut; }
	const float GetEtaOut() const { return m_etaOut; }

	void SetBRDFType(const BRDFType type) { m_materialIndex = static_cast<int>(type); }
	const BRDFType GetBRDFType() const { return static_cast<BRDFType>(m_materialIndex); }

	// 特殊Tex系のallocate
	void AllocateSpecificTexture(std::weak_ptr<DXUTILITY::Device> device, const std::wstring& fileName);

	virtual void WriteData(uint8_t* mapped) override
	{
		m_encodeParameter.param1 = m_ambient;
		m_encodeParameter.param2 = m_albedo; m_encodeParameter.param2.m128_f32[3] = m_albedoScale;
		m_encodeParameter.param3 = m_specular; m_encodeParameter.param2.m128_f32[3] = m_specularPow;
		m_encodeParameter.param4 = m_reflectColor; m_encodeParameter.param4.m128_f32[3] = m_etaIn;
		m_encodeParameter.index = m_materialIndex;
		int TotalSize = GetTotalBufferSize();

		memcpy(mapped, &m_encodeParameter, TotalSize);
		mapped += TotalSize;
	}

	void WriteSpecificTexureForShaderTable(uint8_t* dst);

	// 総メモリサイズ
	virtual const int GetTotalBufferSize() const override { return sizeof(MaterialEncodeParameter); }

protected:
	struct MaterialEncodeParameter
	{
		XMVECTOR param1, param2, param3, param4;
		int index;
	};

protected:
	// 必要パラメータ
	XMVECTOR m_ambient;
	XMVECTOR m_albedo;
	XMVECTOR m_specular;
	XMVECTOR m_reflectColor;
	float m_albedoScale, m_specularPow;
	float m_etaIn, m_etaOut;
	int m_materialIndex;

	// 特殊Tex系
	std::vector<DXUTILITY::TextureResource> m_specialTexture;

	// Encode用のデータ
	MaterialEncodeParameter m_encodeParameter;
};