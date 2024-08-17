#pragma once
#pragma once
#include <vector>
#include <memory>
#include "../Utility/DXUtility.h"
#include "../Utility/TypeUtility.h"
#include "../Utility/MathUtility.h"

class Sampler
{
public:
	Sampler();
	Sampler(const int num);
	~Sampler();

	virtual void Setup(std::weak_ptr<DXUTILITY::Device> device);
	virtual void Release();

	void Update();

	DXUTILITY::Descriptor GetNoiseDescriptor() const { return m_noiseSRV; }
	DXUTILITY::Descriptor GetSeedDescriptor() const { return m_seedUAV; }

protected:
	virtual void GenerateSamples() {};
	
	// シャッフル関係のUtil
	void ShuffleXCoordinates();
	void ShuffleYCoordinates();

	//convert disk from square
	void MapSamplesToUnitDisk();

	//convert hemisphere from spuare
	void MapSamplesToHemisphere(const float p);

protected:
	int m_numSamples;
	std::vector<DirectX::XMFLOAT2> m_sampleList;
	std::vector<DirectX::XMFLOAT2> m_diskSampleList;
	std::vector<DirectX::XMFLOAT3> m_hemisphereSampleList;

	// Noise用データ
	ComPtr<ID3D12Resource> m_noiseResource;
	DXUTILITY::Descriptor m_noiseSRV;

	// Seed用データ
	ComPtr<ID3D12Resource> m_seedResource;
	DXUTILITY::Descriptor m_seedUAV;
};