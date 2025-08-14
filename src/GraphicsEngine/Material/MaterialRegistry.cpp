#include "MaterialRegistry.h"

namespace Graphics
{
	MaterialRegistry::MaterialRegistry(GfxDevice* device, MaterialCache* materialCache)
		: _device(device),
		_materialCache(materialCache)
	{
	}

	void MaterialRegistry::Register(const std::string& key, const MaterialDesc& desc)
	{
		auto material = std::make_unique<Material>();
		material->SetPipelineState(desc, _materialCache->GetOrCreate(*_device, desc));
		_materials[key] = std::move(material);
	}

	Material* MaterialRegistry::Get(const std::string key)
	{
		auto it = _materials.find(key);
		if (it != _materials.end())
		{
			return it->second.get();
		}

		return nullptr;
	}
}