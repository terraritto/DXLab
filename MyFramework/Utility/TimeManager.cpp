#include "TimeManager.h"

void TimeManager::Update()
{
	// 現在の時間を計測して差分を計算
	m_current = std::chrono::system_clock::now();
	m_deltaTime = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(m_current - m_prev).count() / 1000.0);

	// 更新
	m_prev = m_current;
}
