#include "InputManager.h"

void MouseController::CalculateMoveCursor(const DirectX::XMINT2 pos)
{
	// ·•ªŒvZ
	m_dxdy = DirectX::XMINT2(pos.x - m_prevPosition.x, pos.y - m_prevPosition.y);
	m_prevPosition = pos;
}

void InputManager::UpdateInput()
{
	const auto fdx = float(m_mouseController->GetDifference().x) / m_displaySize.x;
	const auto fdy = float(m_mouseController->GetDifference().y) / m_displaySize.y;
	const auto fdxdy = DirectX::XMFLOAT2(fdx, fdy);

	const auto fposx = float(m_mouseController->GetPosition().x) / m_displaySize.x;
	const auto fposy = (m_mouseController->GetPosition().y) / m_displaySize.y;
	const auto fposxy = DirectX::XMFLOAT2(fposx, fposy);

	m_isMouseMove = false; m_isKeyboardInput = false;

	if (m_keyboardController->IsKeyDown())
	{

		for (auto processor : m_inputProcessorList)
		{
			processor->OnKeyDown(m_keyboardController->GetKey());
		}

		m_isKeyboardInput = true;
		return;
	}

	if (m_mouseController->IsMouseMove())
	{
		for (auto processor : m_inputProcessorList)
		{
			processor->OnMouseMove(fdxdy);
		}

		m_isMouseMove = true;
		return;
	}

	if (m_mouseController->IsMouseDown())
	{
		for (auto processor : m_inputProcessorList)
		{
			processor->OnMouseDown(fposxy, m_mouseController->GetMouseState());
		}
		return;
	}

	if (m_mouseController->IsMouseUp())
	{
		for (auto processor : m_inputProcessorList)
		{
			processor->OnMouseUp(fposxy, m_mouseController->GetMouseState());
		}
		return;
	}
}
