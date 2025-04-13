#include <windows.h>

#include "Application.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Application app;
	if (!app.Init(hInstance))
	{
		return -1;
	}

	app.Run();
	app.Terminate();

	return 0;
}