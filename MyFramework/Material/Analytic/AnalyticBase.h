#pragma once
#include "../SelfDefMaterial.h"

class AnalyticBase : public SelfDefMaterial
{
public:
	AnalyticBase() : SelfDefMaterial(){}

	void SetAABBCenter(const XMVECTOR& center)
	{
		m_aabbCenter = center;
	}

	virtual UINT WriteMaterialParamForShaderTable(uint8_t* dst, int index = 0);

	virtual void AllocateConstantBuffer(std::weak_ptr<DXUTILITY::Device> Device) override;
	virtual int GetAnalyticSize() = 0;
	virtual void WriteDataForAnalytic(uint8_t* mapped) = 0;

	virtual void WriteDataFromIndex(int index)
	{
		MaterialBase::WriteDataFromIndex(index);

		auto constantBuffer = m_analyticCB.Get(index);
		void* dst = nullptr;

		// 書き込み
		constantBuffer->Map(0, nullptr, &dst);
		if (dst)
		{
			WriteDataForAnalytic(static_cast<uint8_t*>(dst));
			constantBuffer->Unmap(0, nullptr);
		}
	}

protected:
	XMVECTOR m_aabbCenter;

	// Analytivのコンスタントバッファ
	DXUTILITY::DynamicConstantBuffer m_analyticCB;
};
