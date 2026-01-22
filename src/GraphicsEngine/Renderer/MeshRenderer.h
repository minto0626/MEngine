#pragma once
#include "Renderer.h"
#include "Mesh/Mesh.h"
#include "Material/Material.h"
#include "Resources/ConstantBuffer.h"

#include <memory>

namespace Graphics
{
	class MeshRenderer : public Renderer
	{
	private:
		Mesh* _mesh;
		UINT _transformCBRootParamIndex;
		std::shared_ptr<ConstantBuffer> _transformCB;

	public:
		MeshRenderer(class GameObject* owner, int updateOrder = 100);
        ~MeshRenderer();

		void SetMesh(Mesh* mesh) { _mesh = mesh; };
		void SetMaterial(Material* material) override;

		void Draw(GfxCommandContext* commandContext) override;

	};
}
