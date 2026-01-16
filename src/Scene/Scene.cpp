#include "Scene.h"
#include "Scene/Light.h"

void Scene::Init(Graphics::GraphicsEngine* graphicsEngine)
{
	_graphicsEngine = graphicsEngine;
    _graphicsEngine->InitSceneConstantBuffers(sizeof(CanvasConstantBuffer), sizeof(SceneConstantBuffer));
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

    CanvasConstantBuffer _canvasCB;
    _canvasCB.viewProjectionMatrix = _camera2D->GetViewProjectionMatrix();
    _graphicsEngine->UpdateCanvasConstantBuffer(&_canvasCB, sizeof(_canvasCB));

    // シーン共通の定数バッファを更新
    SceneConstantBuffer _sceneCB;
    _sceneCB.camera.viewMatrix = _camera3D->GetViewMatrix();
    _sceneCB.camera.projectionMatrix = _camera3D->GetProjectionMatrix();
    _sceneCB.camera.cameraPosition = _camera3D->GetGameObject()->GetTransform()->GetPos();
    Matrix lightView, lightProjection;
    Vector3 lightVector = _light->GetGameObject()->GetTransform()->GetForward().Normalized() * -1;
    Vector3 eyePos = _camera3D->GetGameObject()->GetTransform()->GetPos();
    Vector3 targetPos = eyePos + _camera3D->GetGameObject()->GetTransform()->GetForward().Normalized();
    float distance = 6.3f;
    Vector3 lightPos = targetPos + lightVector * distance;
    Vector3 up(0, 1, 0);
    lightView.MakeLookAt(lightPos, targetPos, up);
    lightProjection.MakeOrthographicMatrix(40.0f, 40.0f, .001f, 100.0f);
    _sceneCB.light.lightViewMatrix = lightView * lightProjection;
    _sceneCB.light.lightDirection = _light->GetGameObject()->GetTransform()->GetForward();
    _graphicsEngine->UpdateSceneConstantBuffer(&_sceneCB, sizeof(_sceneCB));

    _graphicsEngine->Render(this);
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
