#include "Scene.h"

void Scene::Init(Graphics::GraphicsEngine* graphicsEngine)
{
	_graphicsEngine = graphicsEngine;
}

void Scene::Update(float deltaTime)
{
	UpdateGameObjects(deltaTime);
	Draw();
}

void Scene::UpdateGameObjects(float deltaTime)
{
	_isUpdating = true;
	for (auto& gameObject : _gameObjects)
	{
		gameObject->Update(deltaTime);
	}
	_isUpdating = false;

	// 保留中のゲームオブジェクトを追加
	for (auto& pending : _pendingGameObjects)
	{
		_gameObjects.push_back(std::move(pending));
	}
	_pendingGameObjects.clear();

	// 死んでいるゲームオブジェクトを削除
	auto iter = _gameObjects.begin();
	while (iter != _gameObjects.end())
	{
		if ((*iter)->GetState() == GameObject::State::Dead)
		{
			iter = _gameObjects.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

void Scene::Draw()
{
	// 描画はここでは行わない
	// 描画はGraphicsEngine側で行う
}

void Scene::AddGameObject(std::unique_ptr<GameObject> gameObject)
{
	if (_isUpdating)
	{
		_pendingGameObjects.push_back(std::move(gameObject));
	}
	else
	{
		_gameObjects.push_back(std::move(gameObject));
	}
}

void Scene::RemoveGameObject(GameObject* gameObject)
{
	auto pred = [gameObject](const std::unique_ptr<GameObject>& obj) { return obj.get() == gameObject; };

	auto iter = std::find_if(_gameObjects.begin(), _gameObjects.end(), pred);
	if (iter != _gameObjects.end())
	{
		_gameObjects.erase(iter);
	}
	else
	{
		iter = std::find_if(_pendingGameObjects.begin(), _pendingGameObjects.end(), pred);
		if (iter != _pendingGameObjects.end())
		{
			_pendingGameObjects.erase(iter);
		}
	}
}

void Scene::AddMeshRenderer(Graphics::MeshRenderer* meshRenderer)
{
    _meshRenderers.push_back(meshRenderer);
}

void Scene::RemoveMeshRenderer(Graphics::MeshRenderer* meshRenderer)
{
    auto iter = std::find(_meshRenderers.begin(), _meshRenderers.end(), meshRenderer);
    if (iter != _meshRenderers.end())
    {
        _meshRenderers.erase(iter);
    }
}

void Scene::AddSpriteRenderer(Graphics::SpriteRenderer* spriteRenderer)
{
    _spriteRenderers.push_back(spriteRenderer);
}

void Scene::RemoveSpriteRenderer(Graphics::SpriteRenderer* spriteRenderer)
{
    auto iter = std::find(_spriteRenderers.begin(), _spriteRenderers.end(), spriteRenderer);
    if (iter != _spriteRenderers.end())
    {
        _spriteRenderers.erase(iter);
    }
}

void Scene::Render(Camera* camera2D, Camera* camera3D, Light* light)
{
    for (auto& spriteRenderer : _spriteRenderers)
    {
        if (!spriteRenderer->IsEnabled())
        {
            continue;
        }
        if (spriteRenderer->GetGameObject()->GetState() != GameObject::State::Active)
        {
            continue;
        }
        _graphicsEngine->RegisterSpriteRenderer(spriteRenderer);
    }

    for (auto& meshRenderer : _meshRenderers)
    {
        if (!meshRenderer->IsEnabled())
        {
            continue;
        }
        if (meshRenderer->GetGameObject()->GetState() != GameObject::State::Active)
        {
            continue;
        }
        _graphicsEngine->RegisterMeshRenderer(meshRenderer);
    }

    _graphicsEngine->Render(this, camera2D, camera3D, light);
}

GameObject* Scene::CreateGameObject(const std::string& name)
{
	auto gameObject = std::make_unique<GameObject>(this, name);
	auto ptr = gameObject.get();
	AddGameObject(std::move(gameObject));
	return ptr;
}

const std::vector<std::unique_ptr<GameObject>>& Scene::GetAllGameObjects() const
{
    return _gameObjects;
}
