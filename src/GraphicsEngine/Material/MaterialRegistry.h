#pragma once
#include <unordered_map>
#include <memory>
#include <string>

#include "Material.h"
#include "MaterialCache.h"
#include "Core/GfxDevice.h"

namespace Graphics
{
	class MaterialRegistry
	{
	private:
		GfxDevice* _device;
		MaterialCache* _materialCache;
		std::unordered_map<std::string, std::unique_ptr<Material>> _materials;

        MaterialDesc LoadFromFile(const std::string& filePath);

	public:
		MaterialRegistry(GfxDevice* device, MaterialCache* materialCache);

		void Register(const std::string& key, const MaterialDesc& desc);
		Material* Get(const std::string key);

	};
}
