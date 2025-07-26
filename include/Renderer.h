#pragma once
#include "Mesh.h"
#include "Material.h"
#include "GfxCommandContext.h"

namespace Graphics
{
	class Renderer
	{
	public:
		virtual ~Renderer() = default;
		virtual void Draw(GfxCommandContext* commandContext) = 0;

	};
}