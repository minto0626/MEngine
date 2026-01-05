#pragma once
#include "GraphicsEngine.h"
#include "Utility/GameTime.h"
#include "Input/InputSystem.h"
#include "Scene/Scene.h"
#include "GUI/GUISystem.h"

#include <vector>
#include <memory>

class MEngine
{
private:

	HWND _hwnd;
	Graphics::GraphicsEngine graphicsEngine;
    static GUISystem guiSystem;

	std::unique_ptr<Scene> _scene;
	InputSystem input;
	Engine::Time time;

	class Camera* camera2D;
	class Camera* camera3D;
    class Light* directionalLight;

    std::vector<GameObject*> sample_objects;

public:
	~MEngine();

	bool Init(HWND hwnd, HINSTANCE hInstance, SIZE& windowSize);
	void Update();
	void Draw();

    static GUISystem* GUI() { return &guiSystem; }

    static inline const std::string CacheDirectory = "cache/";
};
