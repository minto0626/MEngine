#include "MaterialCache.h"

namespace Graphics
{
	MaterialCache::MaterialCache(RootSignatureRegistry* rootSignatureRegistry)
		: _rootSignatureRegistry(rootSignatureRegistry)
	{
	}

	PipelineState* MaterialCache::GetOrCreate(const GfxDevice& device, const MaterialDesc& desc)
	{
		auto it = _cache.find(desc);
		if (it != _cache.end())
		{
			return it->second.get();
		}

		auto pso = std::make_unique<PipelineState>();
		pso->CreateFromDesc(device, desc, *_rootSignatureRegistry); // MaterialDesc から PSO を生成
		PipelineState* ptr = pso.get();
		_cache[desc] = std::move(pso);
		return ptr;
	}
}