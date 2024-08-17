#include "Sampler.h"
#include <random>
#include <stdexcept>

Sampler::Sampler() 
	: m_numSamples(65536)
{
}

Sampler::Sampler(const int num)
	: m_numSamples(num)
{
}

Sampler::~Sampler()
{
}

void Sampler::Setup(std::weak_ptr<DXUTILITY::Device> device)
{
	// noise—pˆÓ
	const auto noiseSize = m_sampleList.size() * sizeof(XMFLOAT2);
	m_noiseResource = DXUTILITY::CreateConstantBuffer(device, noiseSize);
	m_noiseSRV = DXUTILITY::CreateStructuredSRV(device, m_noiseResource, m_sampleList.size(), 0, sizeof(XMFLOAT2));

	void* dst = nullptr;
	m_noiseResource->Map(0, nullptr, &dst);
	if (dst) {
		memcpy(dst, m_sampleList.data(), noiseSize);
		m_noiseResource->Unmap(0, nullptr);
	}
	else
	{
		throw std::runtime_error("Noise resource allocaton is failed.");
	}

	// seed
	constexpr auto SeedSize = sizeof(uint32_t);
	m_seedResource = DXUTILITY::CreateBufferUAV(device, sizeof(uint32_t), D3D12_RESOURCE_STATE_GENERIC_READ, L"SamplerSeed");
	m_seedUAV = DXUTILITY::CreateStructuredUAV(device, m_seedResource, 1, 0, sizeof(uint32_t));
}

void Sampler::Release()
{
}

void Sampler::Update()
{
	GenerateSamples();

	const auto noiseSize = m_sampleList.size() * sizeof(XMFLOAT2);
	void* dst = nullptr;
	m_noiseResource->Map(0, nullptr, &dst);
	if (dst) {
		memcpy(dst, m_sampleList.data(), noiseSize);
		m_noiseResource->Unmap(0, nullptr);
	}
	else
	{
		throw std::runtime_error("Noise resource allocaton is failed.");
	}
}

void Sampler::ShuffleXCoordinates()
{
	std::random_device rd;
	std::mt19937 mt(rd());

	for (int i = 0; i < m_numSamples - 1; i++)
	{
		const int index = i + 1;
		int target = mt() % m_numSamples;
		float temp = m_sampleList[index].x;
		m_sampleList[index].x = m_sampleList[target].x;
		m_sampleList[target].x = temp;
	}
}

void Sampler::ShuffleYCoordinates()
{
	std::random_device rd;
	std::mt19937 mt(rd());

	for (int i = 0; i < m_numSamples - 1; i++)
	{
		const int index = i + 1;
		int target = mt() % m_numSamples;
		float temp = m_sampleList[index].y;
		m_sampleList[index].y = m_sampleList[target].y;
		m_sampleList[target].y = temp;
	}
}

void Sampler::MapSamplesToUnitDisk()
{
	int size = m_sampleList.size();
	float r, phi;
	DirectX::XMFLOAT2 sp;

	m_diskSampleList.resize(size);

	for (int j = 0; j < size; j++)
	{
		sp.x = 2.0 * m_sampleList[j].x - 1.0;
		sp.y = 2.0 * m_sampleList[j].y - 1.0;

		if (sp.x > -sp.y)
		{
			if (sp.x > sp.y)
			{
				//section 1
				r = sp.x;
				phi = sp.y / sp.x;
			}
			else
			{
				//section 2
				r = sp.y;
				phi = 2 - sp.x / sp.y;
			}
		}
		else
		{
			if (sp.x < sp.y)
			{
				//section 3
				r = -sp.x;
				phi = 4 + sp.y / sp.x;
			}
			else
			{
				//section 4
				r = -sp.y;

				if (sp.y != 0.0)
				{
					phi = 6 - sp.x / sp.y;
				}
				else
				{
					phi = 0.0;
				}
			}
		}

		phi *= Utility::PI / 4.0;

		m_diskSampleList[j].x = r * std::cos(phi);
		m_diskSampleList[j].y = r * std::sin(phi);
	}

	m_sampleList.erase(m_sampleList.begin(), m_sampleList.end());
}

void Sampler::MapSamplesToHemisphere(const float e)
{
	int size = m_sampleList.size();
	m_hemisphereSampleList.reserve(m_numSamples);

	for (int j = 0; j < size; j++)
	{
		const float cosPhi = std::cos(2.0 * Utility::PI * m_sampleList[j].x);
		const float sinPhi = std::sin(2.0 * Utility::PI * m_sampleList[j].x);
		const float cosTheta = std::pow((1.0 - m_sampleList[j].y), 1.0 / (e + 1.0));
		float sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);
		float pu = sinTheta * cosPhi;
		float pv = sinTheta * sinPhi;
		float pw = cosTheta;

		m_hemisphereSampleList.emplace_back(XMFLOAT3(pu, pv, pw));
	}
}
