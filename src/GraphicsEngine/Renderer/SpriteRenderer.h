#pragma once
#include "Renderer.h"
#include "Sprite/Sprite.h"
#include "Material/Material.h"
#include "Resources/ConstantBuffer.h"

namespace Graphics
{
	class SpriteRenderer : public Renderer
	{
	private:
		Sprite* _sprite;
		UINT _transformCBRootParamIndex;
		ConstantBuffer* _transformCB;

	public:
		SpriteRenderer(class GameObject* owner, int updateOrder = 100);
        ~SpriteRenderer();

		void SetSprite(Sprite* sprite) { _sprite = sprite; };
		void SetMaterial(Material* material) override;

		void Draw(GfxCommandContext* commandContext, Camera* camera) override;

	};
}
