#pragma once
#include "MaterialBase.h"
#include "../Utility/DXUtility.h"
#include <map>
#include <vector>

class MaterialManager
{
private:
	MaterialManager() = default;
	~MaterialManager() = default;

public:
	// singletonのための実装
	MaterialManager(const MaterialManager&) = delete;
	MaterialManager& operator=(const MaterialManager&) = delete;
	MaterialManager(MaterialManager&&) = delete;
	MaterialManager& operator=(MaterialManager&&) = delete;

	static MaterialManager& GetInstance()
	{
		static MaterialManager instance;
		return instance;
	}

public:
	void Setup(std::weak_ptr<DXUTILITY::Device> device);
	void Uninitialize(std::weak_ptr<DXUTILITY::Device> device);

	int AddMaterial(std::unique_ptr<MaterialBase>&& material);

	std::unique_ptr<MaterialBase>& GetMaterial(int index);
	int GetMaterialTail() const;

	void WriteAllMaterialData();

	// Descriptorの用意
	void WriteDescriptorData();

	// Materialの更新処理
	void UpdateMaterialData(const int index);

protected:
	std::vector<std::unique_ptr<MaterialBase>> m_materialList;

	std::weak_ptr<DXUTILITY::Device> m_device;
};
