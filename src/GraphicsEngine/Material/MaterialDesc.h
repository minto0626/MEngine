#pragma once
#include "Pipeline/StateFactory.h"
#include "Pipeline/RootSignatureDesc.h"
#include "Pipeline/InputLayoutHelper.h"

#include <string>
#include <vector>

namespace Graphics
{
	struct MaterialDesc
	{
		std::wstring vertexShaderPath;
		std::wstring pixelShaderPath;
		std::vector<InputLayoutHelper::InputElement> inputElements;
        std::vector<DXGI_FORMAT> rtvFormats;
		RootSignatureDesc rootSignatureDesc;
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
