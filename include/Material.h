#pragma once
#include "GfxCommandContext.h"
#include "PipelineState.h"
#include "ConstantBuffer.h"
#include "Texture.h"

namespace Graphics
{
	class Material
	{
	private:
		PipelineState* _pipelineState = nullptr;

		ConstantBuffer* _constantBuffer;
		Texture* _texture;

	public:
		void SetPipelineState(PipelineState* pipelineState) { _pipelineState = pipelineState; }
		PipelineState* GetPipelineState() const { return _pipelineState; }
		void SetConstantBuffer(ConstantBuffer* constantBuffer) { _constantBuffer = constantBuffer; }
		void SetTexture(Texture* texture) { _texture = texture; }

		void UploadConstantBuffer(const void* data, UINT size);
		void Bind(GfxCommandContext& commandContext);
	};
}