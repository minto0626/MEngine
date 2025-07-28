#pragma once
#include "Mesh/Mesh.h"
#include "Material/Material.h"
#include "Core/GfxCommandContext.h"

namespace Graphics
{
	class Renderer
	{
	public:
		virtual ~Renderer() = default;
		virtual void Draw(GfxCommandContext* commandContext) = 0;

	};
}