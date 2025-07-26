#include "MeshRenderer.h"

namespace Graphics
{
	MeshRenderer::MeshRenderer(Mesh* mesh, Material* material)
		:_mesh(mesh),
		_material(material)
	{
	}

	void MeshRenderer::Draw(GfxCommandContext* commandContext)
	{
		_material->Bind(*commandContext);
		_mesh->Draw(*commandContext);
	}
}