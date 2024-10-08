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
	// singletonのための実装
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
	// 登録
	void RegisterAreaSphereLight(std::unique_ptr<SphereAreaLightData>&& light);

	// ライトのメモリ用意
	void Initialize(std::weak_ptr<DXUTILITY::Device> device);
	void Uninitialize(std::weak_ptr<DXUTILITY::Device> device);

	// CBの反映
	void UpdateData(int index);

	// 取得
	std::vector<std::unique_ptr<SphereAreaLightData>>& GetLights() { return m_areaLightList; }
	int GetLightNum() const { return m_areaLightList.size(); }
	DXUTILITY::Descriptor GetDescriptor(int frameIndex);

protected:
	std::vector<std::unique_ptr<SphereAreaLightData>> m_areaLightList;
	DXUTILITY::DynamicBuffer m_areaLightCB;
};