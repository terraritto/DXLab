#pragma once
#include "../../MyFramework/PSO/PostPSO.h"

struct RadialBlurData
{
	float m_sampleCount;
	float m_strength;
	float m_uv[2];

	RadialBlurData()
		: m_sampleCount(4.0f)
		, m_strength(0.5f)
		, m_uv()
	{
		m_uv[0] = 0.5f; m_uv[1] = 0.5f;
	}
};

class RadialBlurPSO : public PostPSO
{
public:
	RadialBlurPSO(std::weak_ptr<DXUTILITY::Device> device);

	virtual void Update(double deltaTime) override;

	[[nodiscard]]
	virtual int GetCBSize() const { return sizeof(RadialBlurData); }

	[[nodiscard]]
	virtual void WriteCBData() override;

public:
	std::unique_ptr<RadialBlurData> m_data;
};