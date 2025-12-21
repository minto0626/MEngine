#include "RootSignatureRegistry.h"

namespace Graphics
{
	std::shared_ptr<RootSignature> RootSignatureRegistry::GetOrCreate(const GfxDevice& device, const RootSignatureDesc& desc)
	{
		auto it = _cache.find(desc);
		if (it != _cache.end())
		{
			return it->second;
		}

		auto rootSignature = std::make_shared<RootSignature>();
		for (auto& param : desc.params)
		{
			rootSignature->AddDescriptorTable(
				param.name,
				param.numDescriptors,
				param.shaderRegister,
				param.visibility,
				param.type);
		}
		for (auto& sampler : desc.staticSamplers)
		{
			rootSignature->AddStaticSampler(
				sampler.shaderRegister,
				sampler.visibility,
                sampler.addressMode,
                sampler.comparisonFunc,
                sampler.filter);
		}
		rootSignature->Build(device.Get());

		_cache[desc] = rootSignature;
		return rootSignature;
	}
}
