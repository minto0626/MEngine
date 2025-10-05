#pragma once
#include <vector>
#include <string>
#include <memory>

#include "GraphicsEngine.h"
#include "GameObject.h"

class Scene
{
private:
	std::vector<std::unique_ptr<GameObject>> _gameObjects;
	std::vector<std::unique_ptr<GameObject>> _pendingGameObjects;
	bool _isUpdating = false;
	Graphics::GraphicsEngine* _graphicsEngine;

	void UpdateGameObjects(float deltaTime);
	void Draw();

public:
	Scene() = default;
	~Scene() = default;

	void Init(Graphics::GraphicsEngine* graphicsEngine);
	void Update(float deltaTime);
	void AddGameObject(std::unique_ptr<GameObject> gameObject);
	void RemoveGameObject(GameObject* gameObject);
	Graphics::GraphicsEngine* GetGraphicsEngine() const { return _graphicsEngine; };

	GameObject* CreateGameObject(const std::string& name = "GameObject");

};