#pragma once
#include "../../MyFramework/PSO/PostPSO.h"

struct FXAAData
{
	float m_inv[2];
	float m_useFXAA;

	FXAAData()
		: m_inv()
		, m_useFXAA(0.0f)
	{
		m_inv[0] = 0.0f; m_inv[1] = 0.0f;
	}
};

class FXAAPSO : public PostPSO
{
public:
	FXAAPSO(std::weak_ptr<DXUTILITY::Device> device);

	virtual void Update(double deltaTime) override;

	[[nodiscard]]
	virtual int GetCBSize() const { return sizeof(FXAAData); }

	[[nodiscard]]
	virtual void WriteCBData() override;

public:
	std::unique_ptr<FXAAData> m_data;
};