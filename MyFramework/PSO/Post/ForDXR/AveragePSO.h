#pragma once
#include "../../MyFramework/PSO/PostPSO.h"

struct AverageData
{
	int m_spp;

	AverageData()
		: m_spp(1)
	{
	}
};

class AveragePSO : public PostPSO
{
public:
	AveragePSO(std::weak_ptr<DXUTILITY::Device> device);

	virtual void Update(double deltaTime) override;

	[[nodiscard]]
	virtual int GetCBSize() const { return sizeof(AverageData); }

	[[nodiscard]]
	virtual void WriteCBData() override;

	virtual void Updatespp(int spp);

public:
	std::unique_ptr<AverageData> m_data;
};