#pragma once
#include "Material/MaterialDesc.h"
#include "Pipeline/PipelineState.h"
#include "Pipeline/RootSignatureRegistry.h"

#include <unordered_map>
#include <memory>

namespace Graphics
{
	class MaterialCache
	{
	private:
		RootSignatureRegistry* _rootSignatureRegistry;
		std::unordered_map<MaterialDesc, std::unique_ptr<PipelineState>> _cache;

	public:
		MaterialCache(RootSignatureRegistry* rootSignatureRegistry);

		PipelineState* GetOrCreate(const GfxDevice& device, const MaterialDesc& desc);

	};
}