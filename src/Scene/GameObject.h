#pragma once
#include <vector>
#include <string>
#include <memory>

#include "Component.h"

class GameObject
{
public:
	enum class State
	{
		Active,
		Paused,
		Dead,
	};

private:
	std::string _name;
	State _state;
	class Transform* _transform;
	std::vector<std::unique_ptr<Component>> _components;
	class Scene* _scene;

public:
	GameObject(class Scene* scene, const std::string& name = "GameObject");
	~GameObject();

	void Update(float deltaTime);
	void UpdateComponents(float deltaTime);

	template <typename T, typename... Args>
	T* AddComponent(Args&&... args)
	{
		static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");
		auto comp = std::make_unique<T>(this, std::forward<Args>(args)...);
		T* component = comp.get();
		// 更新順にソートして追加
		int myOrder = component->GetUpdateOrder();
		auto iter = _components.begin();
		for (; iter != _components.end(); ++iter)
		{
			if (myOrder < (*iter)->GetUpdateOrder())
			{
				break;
			}
		}
		_components.insert(iter, std::move(comp));
		//_components.push_back(std::move(comp));
		return component;
	}

	template <typename T>
	void RemoveComponent()
	{
		static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");
		for (auto iter = _components.begin(); iter != _components.end(); ++iter)
		{
			T* t = dynamic_cast<T*>(iter->get());
			if (t != nullptr)
			{
				_components.erase(iter);
				return;
			}
		}
	}

	template <typename T>
	T* GetComponent() const
	{
		static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");
		for (auto& component : _components)
		{
			T* t = dynamic_cast<T*>(component.get());
			if (t != nullptr)
			{
				return t;
			}
		}
		return nullptr;
	}

	const std::string& GetName() const { return _name; }
	State GetState() const { return _state; }
	void SetState(State state) { _state = state; }
	class Scene* GetScene() const { return _scene; }
	class Transform* GetTransform() const { return _transform; }

};