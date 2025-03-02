#pragma once
#include <windows.h>

class Application
{
private:
	WNDCLASSW _windowClass;
	bool CreateGameWindow(HINSTANCE& hInstance, WNDCLASSW& wc);

public:
	bool Init(HINSTANCE& hInstance);
	void Run();
	void Terminate();

	Application();
	~Application();
};

