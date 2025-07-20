#pragma once
#include "Mesh.h"
#include "Material.h"
#include "GfxCommandContext.h"

namespace Graphics
{
	class Renderer
	{
	public:
		void Draw(GfxCommandContext* commandContext, Mesh* mesh, Material* material);

	};
}