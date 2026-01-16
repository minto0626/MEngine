#include "SpriteRenderer.h"
#include "Scene/GameObject.h"
#include "Scene/Scene.h"
#include "Scene/Camera.h"

namespace Graphics
{
	SpriteRenderer::SpriteRenderer(
		class GameObject* owner, int updateOrder) :
		Renderer(owner, updateOrder),
		_sprite(nullptr),
		_transformCBRootParamIndex(0),
		_transformCB(nullptr)
	{
        owner->GetScene()->AddSpriteRenderer(this);
	}

    SpriteRenderer::~SpriteRenderer()
    {
        if (_owner != nullptr && _owner->GetScene() != nullptr)
        {
            _owner->GetScene()->RemoveSpriteRenderer(this);
        }
    }

	void SpriteRenderer::SetMaterial(Material* material)
	{
		_material = material;
		if (_material != nullptr && _owner != nullptr && _owner->GetScene() != nullptr)
		{
			_transformCBRootParamIndex = _owner->GetScene()->GetGraphicsEngine()->GetRootParameterIndex("worldMat", *_material);
			_transformCB = _owner->GetScene()->GetGraphicsEngine()->CreateConstantBuffer(sizeof(ScreenConstantBuffer));
		}
	}

	void SpriteRenderer::Draw(GfxCommandContext* commandContext)
	{
		if (_owner != nullptr)
		{
			ScreenConstantBuffer screenCB;
			screenCB.worldMatrix = _owner->GetTransform()->GetWorldMatrix();
			_transformCB->Update(&screenCB, sizeof(screenCB));
		}
		commandContext->SetGraphicsRootDescriptorTable(_transformCBRootParamIndex, _transformCB->GetGPUHandle());
		_sprite->Draw(*commandContext);
	}
}
