#pragma once
#include <memory>
#include <vector>
#include <DirectXMath.h>
#include <Windows.h>

class MouseController
{
public:
	enum class MouseButton
	{
		LBUTTON,
		RBUTTON,
		MBUTTON,
	};

	const DirectX::XMINT2& GetPosition() const
	{
		return m_currentPosition;
	}

	const DirectX::XMINT2& GetDifference() const
	{
		return m_dxdy;
	}

	// ONÇ…Ç»Ç¡ÇƒÇÈÇ‡ÇÃÇéÊìæ
	const bool IsMouseMove() { bool temp = m_isMouseMove; m_isMouseMove = false; return temp; }
	const bool IsMouseDown() { bool temp = m_isMouseDown; m_isMouseDown = false; return temp; }
	const bool IsMouseUp() { bool temp = m_isMouseUp; m_isMouseUp = false; return temp; }

	const MouseButton GetMouseState() const { return m_MouseState; }

	const bool GetMousePressed() const { return m_isMousePressed; }

	friend class Win32Application;

protected:
	// Win32ApplicationÇ©ÇÁê›íËÇ≥ÇÍÇÈÇ‡ÇÃ
	void SetMouseState(const MouseButton& State) { m_MouseState = State; }
	void SetMousePressed(const bool& pressed) { m_isMousePressed = pressed; }
	void SetMousePosition(const DirectX::XMINT2 pos) { m_currentPosition = pos; m_prevPosition = pos; }

	// ç∑ï™åvéZ
	void CalculateMoveCursor(const DirectX::XMINT2 pos);

	bool m_isMouseMove;
	bool m_isMouseDown;
	bool m_isMouseUp;

private:
	MouseButton m_MouseState;
	bool m_isMousePressed;
	DirectX::XMINT2 m_currentPosition;
	DirectX::XMINT2 m_prevPosition;
	DirectX::XMINT2 m_dxdy;
};

class KeyboardController
{
public:
	const bool IsKeyDown() { bool temp = m_isInputKey; m_isInputKey = false; return temp; }
	WPARAM GetKey()const { return m_key; }


	friend class Win32Application;

protected:
	void SetKey(WPARAM param) { m_key = param; }

	WPARAM m_key;
	bool m_isInputKey;
};

// ì¸óÕèàóùÇÇ∑ÇÈÇΩÇﬂÇÃProcessor
class InputProcessor
{
public:
	virtual void OnMouseMove(DirectX::XMFLOAT2 dxdy) {}
	virtual void OnMouseDown(DirectX::XMFLOAT2 pos, MouseController::MouseButton button) {}
	virtual void OnMouseUp(DirectX::XMFLOAT2 pos, MouseController::MouseButton button) {}
	virtual void OnKeyDown(WPARAM param) {}
};

class InputManager
{
public:
	std::shared_ptr<MouseController> m_mouseController;
	std::shared_ptr<KeyboardController> m_keyboardController;

	InputManager()
		: m_mouseController(std::make_shared<MouseController>())
		, m_keyboardController(std::make_shared<KeyboardController>())
		, m_isMouseMove(false)
		, m_isKeyboardInput(false)
	{
	}
	
	// Inputä÷åWÇÃUpdate
	void UpdateInput();

	// ProcessorÇÃìoò^
	void AddInputProcessor(InputProcessor* processor) { m_inputProcessorList.push_back(processor); }

	// âÊñ ÉTÉCÉYÇÃê›íË
	void SetDisplaySize(const DirectX::XMUINT2 size) { m_displaySize = size; }

	bool IsMouseMoved() const { return m_isMouseMove; }
	bool IsKeyboardInput() const { return m_isKeyboardInput; }

protected:
	std::vector<InputProcessor*> m_inputProcessorList;
	DirectX::XMUINT2 m_displaySize;

	bool m_isMouseMove, m_isKeyboardInput;
};
