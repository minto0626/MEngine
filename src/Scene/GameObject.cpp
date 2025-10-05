#include "GameObject.h"
#include "Scene.h"
#include "Transform.h"

GameObject::GameObject(Scene* scene, const std::string& name)
	: _name(name)
	, _state(State::Active)
	, _scene(scene)
{
	// すべてのゲームオブジェクトはTransformコンポーネントを持つ
	_transform = AddComponent<Transform>();
}

GameObject::~GameObject()
{
	for (auto& component : _components)
	{
		component->OnDestroy();
	}

	_components.clear();
}

void GameObject::Update(float deltaTime)
{
	if (_state == State::Active)
	{
		UpdateComponents(deltaTime);
	}
}

void GameObject::UpdateComponents(float deltaTime)
{
	for (auto& component : _components)
	{
		component->Update(deltaTime);
	}
}