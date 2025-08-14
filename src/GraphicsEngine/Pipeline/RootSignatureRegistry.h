#pragma once
#include "RootSignature.h"
#include "RootSignatureDesc.h"
#include "Core/GfxDevice.h"

#include <unordered_map>
#include <memory>

namespace Graphics
{
	class RootSignatureRegistry
	{
	private:
		std::unordered_map<RootSignatureDesc, std::shared_ptr<RootSignature>> _cache;

	public:
		std::shared_ptr<RootSignature> GetOrCreate(const GfxDevice& device, const RootSignatureDesc& desc);

	};
}