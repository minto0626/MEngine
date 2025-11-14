#include "MeshRenderer.h"
#include "Scene/GameObject.h"
#include "Scene/Scene.h"
#include "Scene/Camera.h"

namespace Graphics
{
	MeshRenderer::MeshRenderer(
		class GameObject* owner, int updateOrder) :
		Renderer(owner, updateOrder),
		_mesh(nullptr),
		_transformCBRootParamIndex(0),
		_transformCB(nullptr)
	{
		// 描画前に随時登録することになると思うので、ここでは登録しない
		owner->GetScene()->GetGraphicsEngine()->RegisterMeshRenderer(this);
	}

	void MeshRenderer::SetMaterial(Material* material)
	{
		_material = material;
		if (_material != nullptr && _owner != nullptr && _owner->GetScene() != nullptr)
		{
			_transformCBRootParamIndex = _owner->GetScene()->GetGraphicsEngine()->GetRootParameterIndex("worldMat", *_material);
			_transformCB = _owner->GetScene()->GetGraphicsEngine()->CreateConstantBuffer(sizeof(ObjectConstantBuffer));
		}
	}

	void MeshRenderer::Draw(GfxCommandContext* commandContext, Camera* camera, Light* light)
	{
		if (_owner != nullptr)
		{
            ObjectConstantBuffer objectCB;
            objectCB.worldMatrix = _owner->GetTransform()->GetWorldMatrix();
            _transformCB->Update(&objectCB, sizeof(objectCB));
		}
		commandContext->SetGraphicsRootDescriptorTable(_transformCBRootParamIndex, _transformCB->GetGPUHandle());
		_mesh->Draw(*commandContext);
	}
}
