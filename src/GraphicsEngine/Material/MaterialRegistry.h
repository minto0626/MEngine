#pragma once
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>

#include "Material.h"
#include "MaterialCache.h"
#include "Core/GfxDevice.h"

namespace Graphics
{
    struct ConstantData
    {
        std::string paramName;
        std::vector<float> value;
    };
    struct TextureData
    {
        std::string paramName;
        std::string path;
    };
    struct ShaderParam
    {
        std::vector<ConstantData> constants;
        std::vector<TextureData> textures;
    };

	class MaterialRegistry
	{
	private:
		GfxDevice* _device;
		MaterialCache* _materialCache;
		std::unordered_map<std::string, std::unique_ptr<Material>> _materials;

        void LoadFromFile(const std::string& filePath, MaterialDesc& materialDesc, ShaderParam& shaderParam);

	public:
		MaterialRegistry(GfxDevice* device, MaterialCache* materialCache);

		void Register(const std::string& key, const MaterialDesc& desc);
		Material* Get(const std::string key, ShaderParam& shaderParam);

	};
}
