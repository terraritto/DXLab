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
	// singleton�̂��߂̎���
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
	// �o�^
	void RegisterAreaSphereLight(std::unique_ptr<SphereAreaLightData>&& light);

	// ���C�g�̃������p��
	void Initialize(std::weak_ptr<DXUTILITY::Device> device);
	void Uninitialize(std::weak_ptr<DXUTILITY::Device> device);

	// CB�̔��f
	void UpdateData(int index);

	// �擾
	std::vector<std::unique_ptr<SphereAreaLightData>>& GetLights() { return m_areaLightList; }
	int GetLightNum() const { return m_areaLightList.size(); }
	DXUTILITY::Descriptor GetDescriptor(int frameIndex);

protected:
	std::vector<std::unique_ptr<SphereAreaLightData>> m_areaLightList;
	DXUTILITY::DynamicBuffer m_areaLightCB;
};