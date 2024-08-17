#include <algorithm>
#include "MaterialManager.h"

void MaterialManager::Setup(std::weak_ptr<DXUTILITY::Device> device)
{
	// device�̃L���b�V��
	m_device = device;

	// �}�e���A�����X�g�̏�����
	m_materialList.clear();
}

void MaterialManager::Uninitialize(std::weak_ptr<DXUTILITY::Device> device)
{
	for (auto& material : m_materialList)
	{
		material->m_materialDescriptorCB.UnInitialize(device);
	}

	m_materialList.clear();
}

int MaterialManager::AddMaterial(std::unique_ptr<MaterialBase>&& material)
{
	// �ǉ����s��	
	m_materialList.emplace_back(std::move(material));

	// index��Ԃ��Ă���(Manager�ł�index�Ǘ��͎s��
	return static_cast<int>(m_materialList.size() - 1);
}

std::unique_ptr<MaterialBase>& MaterialManager::GetMaterial(int index)
{
	return m_materialList[index];
}

int MaterialManager::GetMaterialTail() const
{
	return static_cast<int>(m_materialList.size());
}

void MaterialManager::WriteAllMaterialData()
{
	for (auto& material : m_materialList)
	{
		material->AllocateConstantBuffer(m_device);
	}
}

void MaterialManager::WriteDescriptorData()
{
	for (auto& material : m_materialList)
	{
		material->AllocateBuffer(m_device);
	}
}

void MaterialManager::UpdateMaterialData(const int index)
{
	for (auto& material : m_materialList)
	{
		material->WriteDataFromIndex(index);
	}
}
