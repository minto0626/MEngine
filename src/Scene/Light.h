#pragma once
#include "Component.h"
#include "Math/Color.h"

class Light : public Component
{
private:
    Color _color;

public:
    Light(class GameObject* owner, int updateOrder = 100);
    void SetColor(const Color& color) { _color = color; };
    Color GetColor() const { return _color; };

};
