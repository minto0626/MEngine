#include "MeshRenderer.h"

namespace Graphics
{
	MeshRenderer::MeshRenderer(Mesh* mesh, Material* material, UINT transformCBRootParamIndex, ConstantBuffer* transformCB)
		:_mesh(mesh),
		_material(material),
		_transformCBRootParamIndex(transformCBRootParamIndex),
		_transformCB(transformCB)
	{
	}

	void MeshRenderer::Draw(GfxCommandContext* commandContext)
	{
		_material->Bind(*commandContext);
		commandContext->SetGraphicsRootDescriptorTable(_transformCBRootParamIndex, _transformCB->GetGPUHandle());
		_mesh->Draw(*commandContext);
	}
}