#pragma once
#include "Sampler.h"

class PureRandom : public Sampler
{
public:
	PureRandom();
	PureRandom(const int num);
	~PureRandom();

protected:
	virtual void GenerateSamples() override;
};
