#include "MaterialDesc.h"

namespace Graphics
{
	bool MaterialDesc::operator ==(const MaterialDesc& other) const
	{
		return
            shaderDesc == other.shaderDesc &&
			blendPreset == other.blendPreset &&
			rasterizerPreset == other.rasterizerPreset &&
			depthStencilPreset == other.depthStencilPreset;
	}
}

size_t std::hash<Graphics::MaterialDesc>::operator ()(const Graphics::MaterialDesc& desc) const
{
	size_t h = 0;
    h ^= std::hash<Graphics::ShaderDesc>{}(desc.shaderDesc) + 0x9e3779b9 + (h << 6) + (h >> 2);
	h ^= std::hash<int>{}(static_cast<int>(desc.blendPreset)) + 0x9e3779b9 + (h << 6) + (h >> 2);
	h ^= std::hash<int>{}(static_cast<int>(desc.rasterizerPreset)) + 0x9e3779b9 + (h << 6) + (h >> 2);
	h ^= std::hash<int>{}(static_cast<int>(desc.depthStencilPreset)) + 0x9e3779b9 + (h << 6) + (h >> 2);

	return h;
}
