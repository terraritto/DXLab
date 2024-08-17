#include "PureRandom.h"
#include <random>

PureRandom::PureRandom()
	: Sampler()
{
	GenerateSamples();
}

PureRandom::PureRandom(const int num)
	: Sampler(num)
{
	GenerateSamples();
}

PureRandom::~PureRandom()
{
}

void PureRandom::GenerateSamples()
{
	std::random_device seed_gen;
	std::mt19937 engine(seed_gen());
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);

	m_sampleList.clear();

	for (int j = 0; j < m_numSamples; j++)
	{
		XMFLOAT2 v(dist(engine), dist(engine));
		m_sampleList.emplace_back(v);
	}
}
