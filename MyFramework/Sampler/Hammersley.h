#pragma once
#include "Sampler.h"

class Hammersley : public Sampler
{
public:
	Hammersley();
	Hammersley(const int num);
	~Hammersley();

protected:
	float phi(int j);
	virtual void GenerateSamples() override;
};
