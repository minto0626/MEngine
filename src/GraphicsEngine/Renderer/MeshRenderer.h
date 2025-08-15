#pragma once
#include "Renderer.h"
#include "Mesh/Mesh.h"
#include "Material/Material.h"
#include "Resources/ConstantBuffer.h"

namespace Graphics
{
	class MeshRenderer : public Renderer
	{
	private:
		Mesh* _mesh;
		UINT _transformCBRootParamIndex;
		ConstantBuffer* _transformCB;

	public:
		MeshRenderer(Mesh* mesh, Material* material, UINT transformCBRootParamIndex, ConstantBuffer* transformCB);
		void Draw(GfxCommandContext* commandContext) override;

	};
}