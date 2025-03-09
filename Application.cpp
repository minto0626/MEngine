#include "Application.h"
#include "resource.h"

const unsigned int window_width = 1280;
const unsigned int window_height = 720;

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProcW(hWnd, msg, wp, lp);
	}

	return 0;
}

bool Application::CreateGameWindow(HWND& hwnd, HINSTANCE& hInstance, WNDCLASSW& wc)
{
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hInstance = hInstance;
	wc.lpszClassName = L"DirectX12";
	wc.lpfnWndProc = WindowProcedure;
	wc.hIcon = (HICON)LoadImage(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);

	auto rr = RegisterClassW(&wc);
	if (!rr)
	{
		return false;
	}

	RECT wrect = { 0, 0, window_width, window_height };

	hwnd = CreateWindowW(
		wc.lpszClassName,
		L"MEngine",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		200,
		200,
		wrect.right - wrect.left,
		wrect.bottom - wrect.top,
		nullptr,
		nullptr,
		wc.hInstance,
		nullptr);

	return true;
}

bool Application::Init(HINSTANCE& hInstance)
{
	HWND hwnd = {};
	if (!CreateGameWindow(hwnd, hInstance, _windowClass))
	{
		return false;
	}

	SIZE windowSize = { window_width, window_height };

	//MEngine engine;
	_engine.reset(new MEngine());
	if (!_engine->Init(hwnd, windowSize))
	{
		return false;
	}

	return true;
}

void Application::Run()
{
	MSG msg = {};
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		_engine->Update();
		_engine->Draw();
	}
}

void Application::Terminate()
{
	UnregisterClassW(_windowClass.lpszClassName, _windowClass.hInstance);
}

Application::Application() :
	_windowClass()
{
}

Application::~Application()
{
}
