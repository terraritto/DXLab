#include "FileUtility.h"
#include <fstream>

void HlslDiffObserver::Update()
{
	// ������
	m_currentShader.clear();
	m_isDiffDetected = false;

	// �t�@�C���ǂݍ���
	std::ifstream readFile;
	std::string readline;
	readFile.open(m_pathName, std::ios::in);

	while (!readFile.eof())
	{
		std::getline(readFile, readline);
		m_currentShader += readline;
	}

	// ��̏ꍇ�͏���Ȃ̂ŏ�������
	if (m_prevShader.empty())
	{
		m_prevShader = m_currentShader;
		return;
	}

	// �ύX�����m�����ꍇ�͏�������
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
