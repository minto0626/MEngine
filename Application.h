#pragma once
#include <windows.h>

#include <memory>

#include "MEngine.h"

class Application
{
private:
	WNDCLASSW _windowClass;
	bool CreateGameWindow(HWND& hwnd, HINSTANCE& hInstance, WNDCLASSW& wc);

	std::shared_ptr<MEngine> _engine;

public:
	bool Init(HINSTANCE& hInstance);
	void Run();
	void Terminate();

	Application();
	~Application();
};

