#pragma once
#include "Pipeline/InputLayoutHelper.h"
#include "Pipeline/RootSignatureDesc.h"

#include <string>
#include <vector>

namespace Graphics
{
    struct ShaderDesc
    {
        std::wstring vertexShaderPath;
        std::wstring pixelShaderPath;
        RootSignatureDesc rootSignatureDesc;
        std::vector<InputLayoutHelper::InputElement> inputElements;
        std::vector<DXGI_FORMAT> rtvFormats;

        bool operator ==(const ShaderDesc& other) const
        {
            return
                vertexShaderPath == other.vertexShaderPath &&
                pixelShaderPath == other.pixelShaderPath &&
                rootSignatureDesc == other.rootSignatureDesc &&
                inputElements == other.inputElements &&
                rtvFormats == other.rtvFormats;
        }
    };
}

namespace std
{
    template<>
    struct hash<Graphics::ShaderDesc>
    {
        size_t operator ()(const Graphics::ShaderDesc& desc) const
        {
            size_t h = 0;
            h ^= std::hash<std::wstring>{}(desc.vertexShaderPath) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<std::wstring>{}(desc.pixelShaderPath) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<Graphics::RootSignatureDesc>{}(desc.rootSignatureDesc) + 0x9e3779b9 + (h << 6) + (h >> 2);
            for (auto& element : desc.inputElements)
            {
                h ^= std::hash<InputLayoutHelper::InputElement>{}(element) + 0x9e3779b9 + (h << 6) + (h >> 2);
            }
            for (auto& format : desc.rtvFormats)
            {
                h ^= std::hash<DXGI_FORMAT>{}(format) + 0x9e3779b9 + (h << 6) + (h >> 2);
            }
            return h;
        }
    };
}
