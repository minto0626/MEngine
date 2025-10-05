#include "Renderer.h"
#include "Scene/GameObject.h"

namespace Graphics
{
	Renderer::Renderer(class GameObject* owner, int updateOrder)
		: Component(owner, updateOrder)
		, _material(nullptr)
	{
	}
}