#pragma once
#include "Core/GfxCommandContext.h"
#include "Pipeline/PipelineState.h"
#include "Resources/ConstantBuffer.h"
#include "Resources/Texture.h"
#include "MaterialDesc.h"

#include <unordered_map>
#include <memory>

namespace Graphics
{
	class Material
	{
	private:
		UINT64 _instanceID;
		MaterialDesc _desc;
		PipelineState* _pipelineState = nullptr;
        std::unordered_map<UINT, std::shared_ptr<ConstantBuffer>> _constantBuffers;
		std::unordered_map<UINT, Texture*> _textures;

	public:
		Material(UINT64 instanceID);

		UINT64 GetInstanceID() const { return _instanceID; };
		void SetPipelineState(const MaterialDesc& desc, PipelineState* pipelineState) { _desc = desc; _pipelineState = pipelineState; }
		const MaterialDesc& GetDesc() const { return _desc; }
		PipelineState* GetPipelineState() const { return _pipelineState; }
		void SetConstantBuffer(UINT rootIndex, std::shared_ptr<ConstantBuffer> constantBuffer);
		void SetTexture(UINT rootIndex, Texture* texture);

		void UploadConstantBuffer(UINT rootIndex, const void* data, UINT size);
		void Bind(GfxCommandContext& commandContext);
	};
}
