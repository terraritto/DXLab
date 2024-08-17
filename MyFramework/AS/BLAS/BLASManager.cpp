#include "BLASManager.h"
#include "BLASInterface.h"

void BLASManager::AddBLAS(std::string name, std::unique_ptr<BLASInterface> blas)
{
	// Container�ɒǉ����s��
	// unique�œn���̂ŁA�n���ꂽ���̂̕��͖����ɂȂ�
	m_blasContainer.emplace(name, std::move(blas));
}

void BLASManager::Initialize()
{
	std::shared_ptr<DXUTILITY::Device> device = m_device.lock();
	if (device == nullptr) { return; }

	std::vector<BLASInterface*> blasList;

	// ������
	for (auto& data : m_blasContainer)
	{
		data.second->Initialize(m_device);
		blasList.push_back(data.second.get());
	}

	// �R�}���h�̎��s
	if (m_processor != nullptr)
	{
		m_processor->ProcessCommand(blasList, m_device);
	}
}
