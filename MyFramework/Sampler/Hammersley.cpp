#include "Hammersley.h"
#include "../../MyFramework/Utility/TypeUtility.h"

Hammersley::Hammersley()
	: Sampler()
{
	GenerateSamples();
}

Hammersley::Hammersley(const int num)
	: Sampler(num)
{
	GenerateSamples();
}

Hammersley::~Hammersley()
{
}

float Hammersley::phi(int j)
{
	double x = 0.0;
	double f = 0.5;

	while (j) {
		x += f * static_cast<double>(j & 1);
		j /= 2;
		f *= 0.5;
	}

	return static_cast<float>(x);
}

void Hammersley::GenerateSamples()
{
	for (int j = 0; j < m_numSamples; j++)
	{
		XMFLOAT2 pv(static_cast<float>(j) / static_cast<float>(m_numSamples), phi(j));
		m_sampleList.emplace_back(pv);
	}

	ShuffleXCoordinates();
	ShuffleYCoordinates();
	
	// Hemisphere�̍쐬
	MapSamplesToHemisphere(1);
}
