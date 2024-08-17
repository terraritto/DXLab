#include "FileUtility.h"
#include <fstream>

void HlslDiffObserver::Update()
{
	// 初期化
	m_currentShader.clear();
	m_isDiffDetected = false;

	// ファイル読み込み
	std::ifstream readFile;
	std::string readline;
	readFile.open(m_pathName, std::ios::in);

	while (!readFile.eof())
	{
		std::getline(readFile, readline);
		m_currentShader += readline;
	}

	// 空の場合は初回なので書きこむ
	if (m_prevShader.empty())
	{
		m_prevShader = m_currentShader;
		return;
	}

	// 変更を検知した場合は書き込む
	if (m_prevShader != m_currentShader)
	{
		m_isDiffDetected = true;
		m_prevShader = m_currentShader;
	}
}

void DiffNotifier::Execute()
{
	for (auto& observer : m_observerList)
	{
		observer->Update();
	}
}

std::wstring DiffNotifier::GetFirstObserverPath() const
{
	for (auto& observer : m_observerList)
	{
		if (observer->IsDiffDetected())
		{
			return observer->GetPath();
		}
	}

	return std::wstring();
}
