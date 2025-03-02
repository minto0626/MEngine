#include <windows.h>
#include "resource.h"

const unsigned int window_width = 1280;
const unsigned int window_height = 720;

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSW wc = {};
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hInstance = hInstance;
	wc.lpszClassName = L"DirectX12";
	wc.lpfnWndProc = WindowProcedure;
	wc.hIcon = (HICON)LoadImage(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);

	if (!RegisterClassW(&wc))
	{
		return -1;
	}

	RECT wrect = { 0, 0, window_width, window_height };

	CreateWindowW(
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

	MSG msg = {};
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnregisterClassW(wc.lpszClassName, wc.hInstance);

	return 0;
}

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