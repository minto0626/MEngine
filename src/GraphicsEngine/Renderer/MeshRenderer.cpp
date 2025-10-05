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
			_transformCB = _owner->GetScene()->GetGraphicsEngine()->CreateConstantBuffer(sizeof(SceneConstantBuffer));
		}
	}

	void MeshRenderer::Draw(GfxCommandContext* commandContext, Camera* camera)
	{
		if (_owner != nullptr)
		{
			SceneConstantBuffer sceneCB;
			sceneCB.worldMatrix = _owner->GetTransform()->GetWorldMatrix();
			sceneCB.viewMatrix = camera->GetViewMatrix();
			sceneCB.projectionMatrix = camera->GetProjectionMatrix();
			_transformCB->Update(&sceneCB, sizeof(sceneCB));
		}
		commandContext->SetGraphicsRootDescriptorTable(_transformCBRootParamIndex, _transformCB->GetGPUHandle());
		_mesh->Draw(*commandContext);
	}
}