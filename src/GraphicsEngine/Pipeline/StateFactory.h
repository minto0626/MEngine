#pragma once
#include <d3d12.h>
#include <unordered_map>
#include "Core/d3dx12.h"

namespace Graphics
{
	enum class BlendPreset
	{
		AlphaBlend,
		Opaque,
	};

	enum class RasterizerPreset
	{
		CullBack,
		CullNone,
	};

	enum class DepthStencilPreset
	{
		DepthEnable,
		DepthDisable,
	};

    // ブレンドプリセット変換用マップ
    static const std::unordered_map<std::string, BlendPreset> StringToBlendPreset =
    {
        { "opaque", BlendPreset::Opaque },
        { "alpha_blend", BlendPreset::AlphaBlend },
    };

    static const std::unordered_map<std::string, RasterizerPreset> StringToRasterizerPreset =
    {
        { "cull_back", RasterizerPreset::CullBack },
        { "cull_none", RasterizerPreset::CullNone },
    };

    static const std::unordered_map<std::string, DepthStencilPreset> StringToDepthStencilPreset =
    {
        { "depth_enable", DepthStencilPreset::DepthEnable },
        { "depth_disable", DepthStencilPreset::DepthDisable },
    };

	namespace StateFactory
	{
		constexpr D3D12_BLEND_DESC GetBlendState(BlendPreset preset)
		{
			switch (preset)
			{
			case BlendPreset::AlphaBlend:
			{
				D3D12_BLEND_DESC desc{};
				desc.AlphaToCoverageEnable = false;
				desc.IndependentBlendEnable = false;
				const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTraget =
				{
					true, false,
					D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_OP_ADD,
					D3D12_BLEND_ONE, D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_OP_ADD,
					D3D12_LOGIC_OP_NOOP,
					D3D12_COLOR_WRITE_ENABLE_ALL
				};
				desc.RenderTarget[0] = defaultRenderTraget;
				return desc;
			}

			case BlendPreset::Opaque:
			default:
				return CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			}
		}

	 	constexpr D3D12_RASTERIZER_DESC GetRasterizerState(RasterizerPreset preset)
		{
			switch (preset)
			{
			case RasterizerPreset::CullNone:
			{
				auto desc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
				desc.CullMode = D3D12_CULL_MODE_NONE;
				return desc;
			}
			case RasterizerPreset::CullBack:
			default:
				return CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			}
		}

		constexpr D3D12_DEPTH_STENCIL_DESC GetDepthStencilState(DepthStencilPreset preset)
		{
			switch (preset)
			{
			case Graphics::DepthStencilPreset::DepthDisable:
			{
				auto desc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
				desc.DepthEnable = false;
				return desc;
			}
			case Graphics::DepthStencilPreset::DepthEnable:
			default:
				return CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			}
		}

	}
}
