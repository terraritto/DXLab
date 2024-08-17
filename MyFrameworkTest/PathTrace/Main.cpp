#include "PathTraceRenderer.h"

#define WIN32_APPLICATION_IMPLEMENTATION
#define USE_IMGUI
#include "../../MyFramework/Utility/Win32App.h"

int APIENTRY wWinMain
(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE,
	_In_ LPWSTR,
	_In_ int
)
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	PathTraceRenderer theApp(1920, 1080);
	return Win32Application::Run(&theApp, hInstance);
}