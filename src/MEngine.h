#pragma once
#include "GraphicsEngine.h"
#include "Utility/GameTime.h"
#include "Input/InputSystem.h"
#include "Scene/Scene.h"

#include <memory>

class MEngine
{
private:

	HWND _hwnd;
	Graphics::GraphicsEngine graphicsEngine;

	std::unique_ptr<Scene> _scene;
	InputSystem input;
	Engine::Time time;

	class Camera* camera2D;
	class Camera* camera3D;
    class Light* directionalLight;

	GameObject* sample_spriteObject;
	GameObject* sample_meshObject;

public:
	~MEngine();

	bool Init(HWND hwnd, HINSTANCE hInstance, SIZE& windowSize);
	void Update();
	void Draw();
};
