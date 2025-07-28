#pragma once
#include "Core/GfxCommandContext.h"
#include "Pipeline/PipelineState.h"
#include "Resources/ConstantBuffer.h"
#include "Resources/Texture.h"

#include <unordered_map>

namespace Graphics
{
	class Material
	{
	private:
		PipelineState* _pipelineState = nullptr;
		std::unordered_map<UINT, ConstantBuffer*> _constantBuffers;
		std::unordered_map<UINT, Texture*> _textures;

	public:
		void SetPipelineState(PipelineState* pipelineState) { _pipelineState = pipelineState; }
		PipelineState* GetPipelineState() const { return _pipelineState; }
		void SetConstantBuffer(UINT rootIndex, ConstantBuffer* constantBuffer);
		void SetTexture(UINT rootIndex, Texture* texture);

		void UploadConstantBuffer(UINT rootIndex, const void* data, UINT size);
		void Bind(GfxCommandContext& commandContext);
	};
}