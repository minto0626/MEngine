#include "RootSignatureRegistry.h"

namespace Graphics
{
	void RootSignatureRegistry::Register(const std::string& name, std::shared_ptr<RootSignature> signature)
	{
		_signatures[name] = signature;
	}

	std::shared_ptr<RootSignature> RootSignatureRegistry::Get(const std::string& name)
	{
		auto it = _signatures.find(name);
		if (it == _signatures.end())
		{
			return nullptr;
		}

		return it->second;
	}
}