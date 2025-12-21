#pragma once
#include <vector>
#include <d3d12.h>
#include <string>

namespace Graphics
{
	struct RootParamDesc
	{
		std::string name;
		D3D12_DESCRIPTOR_RANGE_TYPE type;
		UINT numDescriptors;
		UINT shaderRegister;
		D3D12_SHADER_VISIBILITY visibility;

		bool operator ==(const RootParamDesc& other) const
		{
			return
				name == other.name &&
				type == other.type &&
				numDescriptors == other.numDescriptors &&
				shaderRegister == other.shaderRegister &&
				visibility == other.visibility;
		}
	};

	struct StaticSamplerDesc
	{
		UINT shaderRegister;
		D3D12_SHADER_VISIBILITY visibility;
        D3D12_TEXTURE_ADDRESS_MODE addressMode;
        D3D12_COMPARISON_FUNC comparisonFunc;
        D3D12_FILTER filter;

		bool operator ==(const StaticSamplerDesc& other) const
		{
			return
				shaderRegister == other.shaderRegister &&
				visibility == other.visibility &&
                addressMode == other.addressMode &&
                comparisonFunc == other.comparisonFunc &&
                filter == other.filter;
		}
	};

	struct RootSignatureDesc
	{
		std::vector<RootParamDesc> params;
		std::vector<StaticSamplerDesc> staticSamplers;

		bool operator ==(const RootSignatureDesc& other) const
		{
			return
				params == other.params &&
				staticSamplers == other.staticSamplers;
		}
	};
}

namespace std
{
	template<>
	struct hash<Graphics::RootParamDesc>
	{
		size_t operator ()(const Graphics::RootParamDesc& p) const
		{
			size_t h = std::hash<std::string>{}(p.name);
			h ^= std::hash<int>{}(static_cast<int>(p.type)) << 1;
			h ^= std::hash<UINT>{}(p.numDescriptors) << 2;
			h ^= std::hash<UINT>{}(p.shaderRegister) << 3;
			h ^= std::hash<int>{}(static_cast<int>(p.visibility)) << 4;
			return h;
		}
	};

	template<>
	struct hash<Graphics::StaticSamplerDesc>
	{
		size_t operator ()(const Graphics::StaticSamplerDesc& s) const
		{
			size_t h = std::hash<UINT>{}(s.shaderRegister);
			h ^= std::hash<int>{}(static_cast<int>(s.visibility)) << 1;
            h ^= std::hash<int>{}(static_cast<int>(s.addressMode)) << 2;
            h ^= std::hash<int>{}(static_cast<int>(s.comparisonFunc)) << 3;
            h ^= std::hash<int>{}(static_cast<int>(s.filter)) << 4;
			return h;
		}
	};

	template<>
	struct hash<Graphics::RootSignatureDesc>
	{
		size_t operator ()(const Graphics::RootSignatureDesc& desc) const
		{
			size_t h = 0;
			for (auto& param : desc.params)
			{
				h ^= std::hash<Graphics::RootParamDesc>{}(param)+0x9e3779b9 + (h << 6) + (h >> 2);
			}
			for (auto& sampler : desc.staticSamplers)
			{
				h ^= std::hash<Graphics::StaticSamplerDesc>{}(sampler)+0x9e3779b9 + (h << 6) + (h >> 2);
			}

			return h;
		}
	};
}
