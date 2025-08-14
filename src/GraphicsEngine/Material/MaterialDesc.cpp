#include "MaterialDesc.h"

namespace Graphics
{
	bool MaterialDesc::operator ==(const MaterialDesc& other) const
	{
		return
			vertexShaderPath == other.vertexShaderPath &&
			pixelShaderPath == other.pixelShaderPath &&
			rootSignatureDesc == other.rootSignatureDesc &&
			blendPreset == other.blendPreset &&
			rasterizerPreset == other.rasterizerPreset &&
			depthStencilPreset == other.depthStencilPreset;
	}
}

size_t std::hash<Graphics::MaterialDesc>::operator ()(const Graphics::MaterialDesc& desc) const
{
	size_t h = std::hash<std::wstring>{}(desc.vertexShaderPath);
	h ^= std::hash<std::wstring>{}(desc.pixelShaderPath) << 1;
	h ^= std::hash<Graphics::RootSignatureDesc>{}(desc.rootSignatureDesc) << 2;
	h ^= std::hash<int>{}(static_cast<int>(desc.blendPreset)) << 3;
	h ^= std::hash<int>{}(static_cast<int>(desc.rasterizerPreset)) << 4;
	h ^= std::hash<int>{}(static_cast<int>(desc.depthStencilPreset)) << 5;
	return h;
}