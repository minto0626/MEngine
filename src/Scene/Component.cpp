#include "Component.h"
#include "GameObject.h"

Component::Component(GameObject* gameObject, int updateOrder)
	: _owner(gameObject)
	, _updateOrder(updateOrder)
{

}

Component::~Component()
{

}