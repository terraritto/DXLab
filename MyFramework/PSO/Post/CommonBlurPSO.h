#pragma once
#include "../../MyFramework/PSO/PostPSO.h"

struct CommonBlurData
{
	float m_texelSize[2];
	float m_sigma;
	int m_index;

	CommonBlurData()
		: m_index(3)
		, m_sigma(1.0)
	{
		m_texelSize[0] = 1.0; m_texelSize[1] = 0.0;
	}
};

class CommonBlurPSO : public PostPSO
{
public:
	CommonBlurPSO(std::weak_ptr<DXUTILITY::Device> device);

	virtual void Update(double deltaTime) override;

	[[nodiscard]]
	virtual int GetCBSize() const { return sizeof(CommonBlurData); }

	[[nodiscard]]
	virtual void WriteCBData() override;

public:
	std::unique_ptr<CommonBlurData> m_data;
};