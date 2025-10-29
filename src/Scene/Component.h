#pragma once
class GameObject;

class Component
{
protected:
	class GameObject* _owner;
	int _updateOrder;

public:
	Component(class GameObject* owner, int updateOrder = 100);
	virtual ~Component();

	virtual void Update(float deltaTime) {};
	virtual void OnEnable() {};
	virtual void OnDisable() {};
	virtual void OnDestroy() {};

	int GetUpdateOrder() const { return _updateOrder; }
	class GameObject* GetGameObject() const { return _owner; }

};
