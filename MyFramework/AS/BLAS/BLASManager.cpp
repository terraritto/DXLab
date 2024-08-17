#include "BLASManager.h"
#include "BLASInterface.h"

void BLASManager::AddBLAS(std::string name, std::unique_ptr<BLASInterface> blas)
{
	// Containerに追加を行う
	// uniqueで渡すので、渡されたものの方は無効になる
	m_blasContainer.emplace(name, std::move(blas));
}

void BLASManager::Initialize()
{
	std::shared_ptr<DXUTILITY::Device> device = m_device.lock();
	if (device == nullptr) { return; }

	std::vector<BLASInterface*> blasList;

	// 初期化
	for (auto& data : m_blasContainer)
	{
		data.second->Initialize(m_device);
		blasList.push_back(data.second.get());
	}

	// コマンドの実行
	if (m_processor != nullptr)
	{
		m_processor->ProcessCommand(blasList, m_device);
	}
}
