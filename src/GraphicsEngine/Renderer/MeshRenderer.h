#pragma once
#include "Renderer.h"
#include "Mesh/Mesh.h"
#include "Material/Material.h"

namespace Graphics
{
	class MeshRenderer : public Renderer
	{
	private:
		Mesh* _mesh;
		Material* _material;

	public:
		MeshRenderer(Mesh* mesh, Material* material);
		void Draw(GfxCommandContext* commandContext) override;

	};
}