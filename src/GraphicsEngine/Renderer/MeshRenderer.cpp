#include "MeshRenderer.h"

namespace Graphics
{
	MeshRenderer::MeshRenderer(Mesh* mesh, Material* material, UINT transformCBRootParamIndex, ConstantBuffer* transformCB) :
		Renderer::Renderer(material),
		_mesh(mesh),
		_transformCBRootParamIndex(transformCBRootParamIndex),
		_transformCB(transformCB)
	{
	}

	void MeshRenderer::Draw(GfxCommandContext* commandContext)
	{
		commandContext->SetGraphicsRootDescriptorTable(_transformCBRootParamIndex, _transformCB->GetGPUHandle());
		_mesh->Draw(*commandContext);
	}
}