#include "MaterialDesc.h"

namespace Graphics
{
	bool MaterialDesc::operator ==(const MaterialDesc& other) const
	{
		return
			vertexShaderPath == other.vertexShaderPath &&
			pixelShaderPath == other.pixelShaderPath &&
			inputElements == other.inputElements &&
            rtvFormats == other.rtvFormats &&
			rootSignatureDesc == other.rootSignatureDesc &&
			blendPreset == other.blendPreset &&
			rasterizerPreset == other.rasterizerPreset &&
			depthStencilPreset == other.depthStencilPreset;
	}
}

size_t std::hash<Graphics::MaterialDesc>::operator ()(const Graphics::MaterialDesc& desc) const
{
	size_t h = 0;
	h ^= std::hash<std::wstring>{}(desc.vertexShaderPath) + 0x9e3779b9 + (h << 6) + (h >> 2);
	h ^= std::hash<std::wstring>{}(desc.pixelShaderPath) + 0x9e3779b9 + (h << 6) + (h >> 2);
	for (auto& element : desc.inputElements)
	{
		h ^= std::hash<InputLayoutHelper::InputElement>{}(element) + 0x9e3779b9 + (h << 6) + (h >> 2);
	}
    for (auto& format : desc.rtvFormats)
    {
        h ^= std::hash<DXGI_FORMAT>{}(format) + 0x9e3779b9 + (h << 6) + (h >> 2);
    }
	h ^= std::hash<Graphics::RootSignatureDesc>{}(desc.rootSignatureDesc) + 0x9e3779b9 + (h << 6) + (h >> 2);
	h ^= std::hash<int>{}(static_cast<int>(desc.blendPreset)) + 0x9e3779b9 + (h << 6) + (h >> 2);
	h ^= std::hash<int>{}(static_cast<int>(desc.rasterizerPreset)) + 0x9e3779b9 + (h << 6) + (h >> 2);
	h ^= std::hash<int>{}(static_cast<int>(desc.depthStencilPreset)) + 0x9e3779b9 + (h << 6) + (h >> 2);

	return h;
}
