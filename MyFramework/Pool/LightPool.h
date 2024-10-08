#pragma once
#include <string>
#include <vector>
#include <memory>
#include "../Light/Light.h"
#include "../Utility/DXUtility.h"

class LightPool
{
private:
	// destructor
	~LightPool();
	LightPool() = default;

public:
	// singletonのための実装
	LightPool(const LightPool&) = delete;
	LightPool& operator=(const LightPool&) = delete;
	LightPool(LightPool&&) = delete;
	LightPool& operator=(LightPool&&) = delete;

	static LightPool& GetInstance()
	{
		static LightPool instance;
		return instance;
	}

public:
	// 登録
	void RegisterLight(std::unique_ptr<Light>&& Light);

	// ライトのメモリ用意
	void Initialize(std::weak_ptr<DXUTILITY::Device> device);
	void Uninitialize(std::weak_ptr<DXUTILITY::Device> device);

	// CBの反映
	void UpdateData(int index);

	// 取得
	std::vector<std::unique_ptr<Light>>& GetLights() { return m_lightList; }
	int GetLightNum() const { return m_lightList.size(); }
	DXUTILITY::Descriptor GetDescriptor(int frameIndex);

protected:
	std::vector<std::unique_ptr<Light>> m_lightList;
	DXUTILITY::DynamicBuffer m_lightCB;
};
