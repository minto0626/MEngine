#include "SpriteRenderer.h"

namespace Graphics
{
	SpriteRenderer::SpriteRenderer(Sprite* sprite, Material* material, UINT transformCBRootParamIndex, ConstantBuffer* transformCB) :
		Renderer::Renderer(material),
		_sprite(sprite),
		_transformCBRootParamIndex(transformCBRootParamIndex),
		_transformCB(transformCB)
	{

	}

	void SpriteRenderer::Draw(GfxCommandContext* commandContext)
	{
		commandContext->SetGraphicsRootDescriptorTable(_transformCBRootParamIndex, _transformCB->GetGPUHandle());
		_sprite->Draw(*commandContext);
	}
}