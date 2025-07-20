#pragma once
#include "GraphicsEngine.h"
#include "InputSystem.h"
#include "GameTime.h"

class MEngine
{
private:

	HWND _hwnd;
	Graphics::GraphicsEngine graphicsEngine;

	InputSystem input;
	Engine::Time time;

public:
	~MEngine();

	bool Init(HWND hwnd, HINSTANCE hInstance, SIZE& windowSize);
	void Update();
	void Draw();
};