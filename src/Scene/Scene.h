#pragma once
#include <vector>
#include <string>
#include <memory>

#include "GraphicsEngine.h"
#include "GameObject.h"
#include "Renderer/MeshRenderer.h"
#include "Renderer/SpriteRenderer.h"

class Scene
{
private:
	std::vector<std::unique_ptr<GameObject>> _gameObjects;
	std::vector<std::unique_ptr<GameObject>> _pendingGameObjects;
	bool _isUpdating = false;
	Graphics::GraphicsEngine* _graphicsEngine;
    std::vector<Graphics::MeshRenderer*> _meshRenderers;
    std::vector<Graphics::SpriteRenderer*> _spriteRenderers;

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
    void AddMeshRenderer(Graphics::MeshRenderer* meshRenderer);
    void RemoveMeshRenderer(Graphics::MeshRenderer* meshRenderer);
    void AddSpriteRenderer(Graphics::SpriteRenderer* spriteRenderer);
    void RemoveSpriteRenderer(Graphics::SpriteRenderer* spriteRenderer);
    void Render(class Camera* camera2D, class Camera* camera3D, class Light* light);

	GameObject* CreateGameObject(const std::string& name = "GameObject");
    const std::vector<std::unique_ptr<GameObject>>& GetAllGameObjects() const;

};
