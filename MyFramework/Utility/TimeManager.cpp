#include "TimeManager.h"

void TimeManager::Update()
{
	// ���݂̎��Ԃ��v�����č������v�Z
	m_current = std::chrono::system_clock::now();
	m_deltaTime = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(m_current - m_prev).count() / 1000.0);

	// �X�V
	m_prev = m_current;
}
