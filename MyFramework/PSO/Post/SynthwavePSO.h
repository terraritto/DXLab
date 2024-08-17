#pragma once
#include "../../MyFramework/PSO/PostPSO.h"

struct SynthwaveData
{
	float m_backColorUpper[4];
	float m_backColorLower[4];
	float m_sunColor[4];
	float m_uvTime[3];

	SynthwaveData()
		: m_backColorUpper()
		, m_backColorLower()
	{
		m_backColorUpper[0] = 0.4; m_backColorUpper[1] = 0.1; m_backColorUpper[2] = 0.7;
		m_backColorUpper[3] = 0.84f;

		m_backColorLower[0] = 0.0; m_backColorLower[1] = 0.38; m_backColorLower[2] = 0.6f;
		m_backColorLower[3] = 0.905f;

		m_sunColor[0] = 1.0; m_sunColor[1] = 0.477; m_sunColor[2] = 0.698;
		m_uvTime[0] = 0.5; m_uvTime[1] = 0.442; m_uvTime[2] = 0.1;
	}
};

class SynthwavePSO : public PostPSO
{
public:
	SynthwavePSO(std::weak_ptr<DXUTILITY::Device> device);

	virtual void Update(double deltaTime) override;

	[[nodiscard]]
	virtual int GetCBSize() const { return sizeof(SynthwaveData); }

	[[nodiscard]]
	virtual void WriteCBData() override;

public:
	std::unique_ptr<SynthwaveData> m_data;
};