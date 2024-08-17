#pragma once
#include <chrono>

class TimeManager
{
public:
	TimeManager()
		: m_deltaTime(0.0)
		, m_current()
		, m_prev()
	{
		m_current = std::chrono::system_clock::now();
		m_prev = std::chrono::system_clock::now();
	}

	void Update();

	double GetDeltaTime() const { return m_deltaTime; }

protected:
	double m_deltaTime;
	std::chrono::system_clock::time_point m_current, m_prev;
};