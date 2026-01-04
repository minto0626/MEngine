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
