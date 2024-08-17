#pragma once
#include "../../MyFramework/PSO/PostPSO.h"

struct ConcentrateData
{
	float m_color[3];
	float m_time;
	float m_speed;
	float m_noiseScale;
	float m_uv[2];
	float m_lineRegion;

	ConcentrateData()
		: m_time(0.0f)
		, m_speed(1.0f)
		, m_noiseScale(40.0f)
		, m_uv()
		, m_lineRegion(0.0f)
	{
		m_uv[0] = 0.5f; m_uv[1] = 0.5f;
		m_color[0] = m_color[1] = m_color[2] = 0.0f;
	}
};

class ConcentrationLinePSO : public PostPSO
{
public:
	ConcentrationLinePSO(std::weak_ptr<DXUTILITY::Device> device);

	virtual void Update(double deltaTime) override;

	[[nodiscard]]
	virtual int GetCBSize() const { return sizeof(ConcentrateData); }

	[[nodiscard]]
	virtual void WriteCBData() override;

public:
	std::unique_ptr<ConcentrateData> m_data;
};