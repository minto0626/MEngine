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
		size_t h = 0;
		h ^= std::hash<Graphics::MaterialDesc>{}(desc)+0x9e3779b9 + (h << 6) + (h >> 2);
		UINT64 instanceID = static_cast<UINT64>(h);

		auto material = std::make_unique<Material>(instanceID);
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