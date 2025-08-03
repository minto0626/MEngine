#pragma once
#include "MaterialDesc.h"
#include "Pipeline/PipelineState.h"

#include <unordered_map>
#include <memory>

namespace Graphics
{
	class MaterialCache
	{
	private:
		std::unordered_map<MaterialDesc, std::unique_ptr<PipelineState>> _cache;

	public:
		PipelineState* GetOrCreate(const MaterialDesc& desc);

	};
}