#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

#include "RenderInterface.h"
#include "InputManager.h"

class RenderInterface;

class Win32Application
{
public:
	static int Run(RenderInterface* app, HINSTANCE hInstance);

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT msgg, WPARAM wp, LPARAM lp);
	static HWND GetHWND() { return m_hWnd; }
private:
	static HWND m_hWnd;
};

#if defined(WIN32_APPLICATION_IMPLEMENTATION)
#include <exception>
#include <stdexcept>
#include <sstream>

#if defined(USE_IMGUI)
#include "../../include/imgui/imgui.h"
#include "../../include/imgui/imgui_impl_win32.h"
#endif

HWND Win32Application::m_hWnd;

int Win32Application::Run(RenderInterface* app, HINSTANCE hInstance)
{
	if (app == nullptr)
	{
		return ERROR_PKINIT_FAILURE;
	}

#if defined(USE_IMGUI)
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
#endif

	try
	{
		WNDCLASSEXW wc{};
		wc.cbSize = sizeof(wc);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.hInstance = hInstance;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.lpfnWndProc = Win32Application::WindowProc;
		wc.lpszClassName = L"RaytracingClass";
		RegisterClassExW(&wc);

		RECT rc{};
		rc.right = LONG(app->GetWidth());
		rc.bottom = LONG(app->GetHeight());

		DWORD dwStyle = WS_OVERLAPPEDWINDOW;
		dwStyle &= ~(WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX);
		AdjustWindowRect(&rc, dwStyle, FALSE);

		m_hWnd = CreateWindowW
		(
			wc.lpszClassName,
			app->GetTitle(),
			dwStyle,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			rc.right - rc.left,
			rc.bottom - rc.top,
			nullptr,
			nullptr,
			hInstance,
			app
		);

#if defined(USE_IMGUI)
		ImGui_ImplWin32_Init(m_hWnd);
#endif

		app->Initialize();

		ShowWindow(m_hWnd, SW_SHOWNORMAL);

		MSG msg{};
		while (msg.message != WM_QUIT)
		{
			if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
		}

		app->Destroy();

#if defined(USE_IMGUI)
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
#endif

		return EXIT_SUCCESS;
	}
	catch (std::exception& e)
	{
		std::ostringstream ss;
		ss << "Exception Occurred.\n";
		ss << e.what() << std::endl;
		OutputDebugStringA(ss.str().c_str());
		app->Destroy();
		return EXIT_FAILURE;
	}
}

#if defined(USE_IMGUI)
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

LRESULT CALLBACK Win32Application::WindowProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	PAINTSTRUCT ps{};
	POINT mousePoint{};
	static POINT lastMousePoint{};
	auto* app = (RenderInterface*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

#if defined(USE_IMGUI)
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wp, lp)) {
		return TRUE;
	}
	auto io = ImGui::GetIO();
#endif

	switch (msg)
	{
	case WM_CREATE:
	{
		auto pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lp);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
	}
	return 0;

	case WM_PAINT:
		if (app)
		{
			app->m_input->UpdateInput(); // inputのUpdateはここで保証
			app->m_time->Update();
			app->Update(app->m_time->GetDeltaTime());
			app->OnRender();
		}
		return 0;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
#if defined(USE_IMGUI)
		if (io.WantCaptureMouse) {
			break;
		}
#endif

		if (app)
		{
			auto btn = MouseController::MouseButton::LBUTTON;
			if (msg == WM_RBUTTONDOWN)
			{
				btn = MouseController::MouseButton::RBUTTON;
			}

			if (msg == WM_MBUTTONDOWN)
			{
				btn = MouseController::MouseButton::MBUTTON;
			}
			SetCapture(hWnd);
			GetCursorPos(&mousePoint);
			ScreenToClient(hWnd, &mousePoint);

			// Inputにデータを渡す
			app->m_input->m_mouseController->SetMouseState(btn);
			app->m_input->m_mouseController->SetMousePressed(true);
			app->m_input->m_mouseController->SetMousePosition(DirectX::XMINT2(int(mousePoint.x), int(mousePoint.y)));
			app->m_input->m_mouseController->m_isMouseDown = true;
		}
		break;

	case WM_MOUSEMOVE:
#if defined(USE_IMGUI)
		if (io.WantCaptureMouse) {
			break;
		}
#endif
	{
		if (app && app->m_input->m_mouseController->GetMousePressed())
		{
			GetCursorPos(&mousePoint);
			ScreenToClient(hWnd, &mousePoint);
			app->m_input->m_mouseController->CalculateMoveCursor(DirectX::XMINT2(int(mousePoint.x), int(mousePoint.y)));
			app->m_input->m_mouseController->m_isMouseMove = true;
		}
	}
		break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
#if defined(USE_IMGUI)
		if (io.WantCaptureMouse) {
			break;
		}
#endif
		if (app)
		{
			ReleaseCapture();

			auto btn = MouseController::MouseButton::LBUTTON;
			if (msg == WM_RBUTTONUP)
			{
				btn = MouseController::MouseButton::RBUTTON;
			}

			if (msg == WM_MBUTTONUP)
			{
				btn = MouseController::MouseButton::MBUTTON;
			}
			GetCursorPos(&mousePoint);
			ScreenToClient(hWnd, &mousePoint);

			// Inputにデータを渡す
			app->m_input->m_mouseController->SetMouseState(btn);
			app->m_input->m_mouseController->SetMousePressed(false);
			app->m_input->m_mouseController->SetMousePosition(DirectX::XMINT2(int(mousePoint.x), int(mousePoint.y)));
			app->m_input->m_mouseController->m_isMouseUp = true;
		}
		break;

	case WM_KEYDOWN:
		if (wp == VK_ESCAPE)
		{
			PostQuitMessage(0);
			return 0;
		}

		app->m_input->m_keyboardController->SetKey(wp);
		app->m_input->m_keyboardController->m_isInputKey = true;
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	default:
		break;
	}
	return DefWindowProcW(hWnd, msg, wp, lp);
}

#endif
