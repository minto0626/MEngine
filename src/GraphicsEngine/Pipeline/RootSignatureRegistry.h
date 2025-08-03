#pragma once
#include "RootSignature.h"

#include <unordered_map>
#include <string>
#include <memory>

namespace Graphics
{
	class RootSignatureRegistry
	{
	private:
		std::unordered_map<std::string, std::shared_ptr<RootSignature>> _signatures;

	public:
		void Register(const std::string& name, std::shared_ptr<RootSignature> signature);
		std::shared_ptr<RootSignature> Get(const std::string& name);

	};
}