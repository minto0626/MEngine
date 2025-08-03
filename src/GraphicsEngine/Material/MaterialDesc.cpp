#include "MaterialDesc.h"

namespace Graphics
{
	bool MaterialDesc::operator ==(const MaterialDesc& other) const
	{
		return
			vertexShaderPath == other.vertexShaderPath &&
			pixelShaderPath == other.pixelShaderPath &&
			rootSignatureName == other.rootSignatureName &&
			blendStateName == other.blendStateName &&
			rasterizerStateName == other.rasterizerStateName &&
			depthStencilStateName == other.depthStencilStateName;
	}
}

size_t std::hash<Graphics::MaterialDesc>::operator ()(const Graphics::MaterialDesc& desc) const
{
	size_t h = std::hash<std::wstring>{}(desc.vertexShaderPath);
	h ^= std::hash<std::wstring>{}(desc.pixelShaderPath) << 1;
	h ^= std::hash<std::string>{}(desc.rootSignatureName) << 2;
	h ^= std::hash<std::string>{}(desc.blendStateName) << 3;
	h ^= std::hash<std::string>{}(desc.rasterizerStateName) << 4;
	h ^= std::hash<std::string>{}(desc.depthStencilStateName) << 5;
	return h;
}