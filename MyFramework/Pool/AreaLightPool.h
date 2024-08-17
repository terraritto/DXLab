#pragma once
#include <string>
#include <vector>
#include <memory>
#include "../Light/Light.h"
#include "../Utility/DXUtility.h"

class AreaLightPool
{
private:
	// destructor
	~AreaLightPool();
	AreaLightPool() = default;

public:
	// singleton‚Ì‚½‚ß‚ÌÀ‘•
	AreaLightPool(const AreaLightPool&) = delete;
	AreaLightPool& operator=(const AreaLightPool&) = delete;
	AreaLightPool(AreaLightPool&&) = delete;
	AreaLightPool& operator=(AreaLightPool&&) = delete;

	static AreaLightPool& GetInstance()
	{
		static AreaLightPool instance;
		return instance;
	}

public:
	// “o˜^
	void RegisterAreaSphereLight(std::unique_ptr<SphereAreaLightData>&& light);

	// ƒ‰ƒCƒg‚Ìƒƒ‚ƒŠ—pˆÓ
	void Initialize(std::weak_ptr<DXUTILITY::Device> device);
	void Uninitialize(std::weak_ptr<DXUTILITY::Device> device);

	// CB‚Ì”½‰f
	void UpdateData(int index);

	// æ“¾
	std::vector<std::unique_ptr<SphereAreaLightData>>& GetLights() { return m_areaLightList; }
	int GetLightNum() const { return m_areaLightList.size(); }
	DXUTILITY::Descriptor GetDescriptor(int frameIndex);

protected:
	std::vector<std::unique_ptr<SphereAreaLightData>> m_areaLightList;
	DXUTILITY::DynamicBuffer m_areaLightCB;
};