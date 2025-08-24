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
		SpriteRenderer(Sprite* sprite, Material* material, UINT transformCBRootParamIndex, ConstantBuffer* transformCB);
		void Draw(GfxCommandContext* commandContext) override;

	};
}