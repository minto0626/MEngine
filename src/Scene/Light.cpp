#include "Light.h"

Light::Light(class GameObject* owner, int updateOrder)
    : Component(owner, updateOrder),
      _color(1.0f, 1.0f, 1.0f, 1.0f)
{
}
