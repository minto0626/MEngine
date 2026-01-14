#pragma once
class GameObject;

class Component
{
protected:
	class GameObject* _owner;
	int _updateOrder;
    bool _enabled = true;

public:
	Component(class GameObject* owner, int updateOrder = 100);
	virtual ~Component();

	virtual void Update(float deltaTime) {};
    virtual void OnEnable() { _enabled = true; };
    virtual void OnDisable() { _enabled = false; };
	virtual void OnDestroy() {};

	int GetUpdateOrder() const { return _updateOrder; }
	class GameObject* GetGameObject() const { return _owner; }
    bool IsEnabled() const { return _enabled; }

};
