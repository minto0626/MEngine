#pragma once
#include "Material/Material.h"
#include "Core/GfxCommandContext.h"
#include "Scene/Component.h"
#include "Scene/Camera.h"

namespace Graphics
{
	class Renderer : public Component
	{
	protected:
		Material* _material;

	public:
		Renderer(class GameObject* owner, int updateOrder = 100);
		virtual ~Renderer() = default;

		virtual void SetMaterial(Material* material) { _material = material; }
		Material* GetMaterial() const { return _material; }

		virtual void Draw(GfxCommandContext* commandContext, Camera* camera) = 0;

	};
}