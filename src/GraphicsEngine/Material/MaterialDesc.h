#pragma once
#include "Pipeline/StateFactory.h"
#include "Shader/ShaderDesc.h"

namespace Graphics
{
	struct MaterialDesc
	{
        ShaderDesc shaderDesc;
		BlendPreset blendPreset;
		RasterizerPreset rasterizerPreset;
		DepthStencilPreset depthStencilPreset;

		bool operator ==(const MaterialDesc & other) const;
	};
}

namespace std
{
	template<>
	struct hash<Graphics::MaterialDesc>
	{
		size_t operator ()(const Graphics::MaterialDesc& desc) const;
	};
}
