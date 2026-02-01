#include "MaterialRegistry.h"
#include "GraphicsEngine/GraphicsEngine.h"
#include <fstream>
#include <Library/nlohmann_json/json.hpp>
using Json = nlohmann::json;

namespace Graphics
{
	MaterialRegistry::MaterialRegistry(GfxDevice* device, MaterialCache* materialCache)
		: _device(device),
		_materialCache(materialCache)
	{
	}

    MaterialDesc MaterialRegistry::LoadFromFile(const std::string& filePath)
    {
        std::ifstream inFile(filePath);
        if (!inFile.is_open())
        {
            assert(0 && "マテリアル定義ファイルが見つかりません");
        }

        Json data;
        inFile >> data;
        inFile.close();

        MaterialDesc desc;
        auto& shaderDesc = desc.shaderDesc;

        std::string shaderPath = data["shader"].get<std::string>();
        std::ifstream shaderDescFile(shaderPath);
        if (!shaderDescFile.is_open())
        {
            assert(0 && "シェーダー定義ファイルが見つかりません");
        }

        Json shader_data;
        shaderDescFile >> shader_data;
        shaderDescFile.close();

        std::string vs_path = shader_data["shaders"]["vs"].get<std::string>();
        std::string ps_path = shader_data["shaders"]["ps"].get<std::string>();
        shaderDesc.vertexShaderPath = std::wstring(vs_path.begin(), vs_path.end());
        shaderDesc.pixelShaderPath = std::wstring(ps_path.begin(), ps_path.end());
        for (const auto& element : shader_data["input_elements"])
        {
            shaderDesc.inputElements.push_back
            (
                InputLayoutHelper::InputElement
                (
                    element["semantic"].get<std::string>(),
                    Graphics::StringToFormat.at(element["format"].get<std::string>())
                )
            );
        }
        for (const auto& formatStr : shader_data["rtv_formats"])
        {
            shaderDesc.rtvFormats.push_back(Graphics::StringToFormat.at(formatStr.get<std::string>()));
        }
        for (const auto& param : shader_data["root_signature"]["params"])
        {
            Graphics::RootParamDesc root_param{};
            root_param.name = param["name"].get<std::string>();
            root_param.type = Graphics::StringToRangeType.at(param["type"].get<std::string>());
            root_param.numDescriptors = param["numDescriptors"].get<UINT>();
            root_param.shaderRegister = param["shaderRegister"].get<UINT>();
            root_param.visibility = Graphics::StringToShaderVisibility.at(param["visibility"].get<std::string>());
            shaderDesc.rootSignatureDesc.params.push_back(root_param);
        }
        for (const auto& sampler : shader_data["root_signature"]["samplers"])
        {
            Graphics::StaticSamplerDesc static_sampler{};
            static_sampler.shaderRegister = sampler["shaderRegister"].get<UINT>();
            static_sampler.visibility = Graphics::StringToShaderVisibility.at(sampler["visibility"].get<std::string>());
            static_sampler.addressMode = Graphics::StringToTextureAddressMode.at(sampler["addressMode"].get<std::string>());
            static_sampler.comparisonFunc = Graphics::StringToComparisonFunc.at(sampler["comparisonFunc"].get<std::string>());
            static_sampler.filter = Graphics::StringToFilter.at(sampler["filter"].get<std::string>());
            shaderDesc.rootSignatureDesc.staticSamplers.push_back(static_sampler);
        }
        shaderDescFile.close();

        desc.blendPreset = StringToBlendPreset.at(data["blend"].get<std::string>());
        desc.rasterizerPreset = StringToRasterizerPreset.at(data["rasterizer"].get<std::string>());
        desc.depthStencilPreset = StringToDepthStencilPreset.at(data["depthStencil"].get<std::string>());

        return desc;
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

        Register(key, LoadFromFile(key));

		return _materials[key].get();
	}
}
