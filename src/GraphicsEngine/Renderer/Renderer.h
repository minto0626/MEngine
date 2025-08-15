#pragma once
#include "Material/Material.h"
#include "Core/GfxCommandContext.h"

namespace Graphics
{
	class Renderer
	{
	protected:
		Material* _material;

	public:
		Renderer(Material* material);
		virtual ~Renderer() = default;

		Material* GetMaterial() const { return _material; };
		virtual void Draw(GfxCommandContext* commandContext) = 0;

	};
}