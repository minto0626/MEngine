#include "TextureLoader.h"
#include "Core/d3dx12.h"
#include "Core/GraphicsContext.h"
#include "DescriptorHeap/DescriptorHeap.h"
#include "Utility/StringUtility.h"

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;

namespace
{
	// ファイル名から拡張子を得る
	// @param path 対象のパス文字列
	// @return 拡張子
	string GetExtension(const string& path)
	{
		size_t idx = path.rfind('.');
		return path.substr(idx + 1, path.length() - idx - 1);
	}
}

void TextureLoader::CreateTextureLoaderTable()
{
	// BMPやPNGなど、Windowsがデフォルトで読める基本的な画像形式
	_loadLamdaTable["sph"]
		= _loadLamdaTable["spa"]
		= _loadLamdaTable["bmp"]
		= _loadLamdaTable["png"]
		= _loadLamdaTable["jpg"]
		= [](const wstring& path, TexMetadata* meta, ScratchImage& image)
		-> HRESULT
		{
			return LoadFromWICFile(path.c_str(), WIC_FLAGS_NONE, meta, image);
		};
	// TGAなどの一部の3Dソフトで使用されているテクスチャファイル形式
	_loadLamdaTable["tga"] = [](const wstring& path, TexMetadata* meta, ScratchImage& image)
		-> HRESULT
		{
			return LoadFromTGAFile(path.c_str(), meta, image);
		};
	// DirectX用の圧縮テクスチャファイル形式
	_loadLamdaTable["dds"] = [](const wstring& path, TexMetadata* meta, ScratchImage& image)
		-> HRESULT
		{
			return LoadFromDDSFile(path.c_str(), DDS_FLAGS_NONE, meta, image);
		};
}

std::shared_ptr<Texture> TextureLoader::CreateTexture(const char* texPath)
{
    auto ext = GetExtension(texPath);
    // 読み込める拡張子が存在しない
    if (_loadLamdaTable.find(ext) == _loadLamdaTable.end())
    {
        return nullptr;
    }

    auto wtexPath = StringUtility::ToWideString(texPath);
    TexMetadata metadata = {};
    ScratchImage scratchImage = {};
    auto result = _loadLamdaTable[ext](wtexPath, &metadata, scratchImage);
    if (FAILED(result))
    {
        return nullptr;
    }

    auto* device = _graphicsContext->device->Get();
    auto* commandList = _graphicsContext->commandContext->GetCommandList();

	// 生データ抽出
    auto image = scratchImage.GetImage(0, 0, 0);

    // GPU用テクスチャを作成
    auto texHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto texDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        metadata.format,
        metadata.width,
        static_cast<UINT>(metadata.height),
        static_cast<UINT16>(metadata.arraySize),
        static_cast<UINT16>(metadata.mipLevels));

    ID3D12Resource* texBuffer = nullptr;
    result = device->CreateCommittedResource(
        &texHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&texBuffer));
    if (FAILED(result))
    {
        return nullptr;
    }

    // 中間アップロード用のバッファを作成
    auto uploadHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    UINT64 uploadBufferSize = GetRequiredIntermediateSize(texBuffer, 0, 1);
    auto uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

    ComPtr<ID3D12Resource> uploadBuffer;
    result = device->CreateCommittedResource(
        &uploadHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &uploadDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(uploadBuffer.ReleaseAndGetAddressOf()));
    if (FAILED(result))
    {
        return nullptr;
    }

    // デフォルトヒープにデータ転送
    D3D12_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pData = image->pixels;
    subresourceData.RowPitch = image->rowPitch;
    subresourceData.SlicePitch = image->slicePitch;
    UpdateSubresources(commandList, texBuffer, uploadBuffer.Get(), 0, 0, 1, &subresourceData);

    _graphicsContext->commandContext->ResourceBarrier(
        texBuffer,
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    _graphicsContext->ExecuteCommand();
    _graphicsContext->WaitGPU();
    _graphicsContext->ResetCommand();

    std::wstring name = L"texture";
    name += wtexPath;
    texBuffer->SetName(name.c_str());

    auto texture = make_shared<Texture>();
    texture->Init(device, _cbvSrvUavHeap, ComPtr<ID3D12Resource>(texBuffer), texDesc.Format);

    return texture;
}

void TextureLoader::Init(Graphics::GraphicsContext* graphicsContext, DescriptorHeap* descHeap)
{
	_graphicsContext = graphicsContext;
    _cbvSrvUavHeap = descHeap;

	CreateTextureLoaderTable();
}

Texture* TextureLoader::GetTexture(const char* texPath)
{
    auto it = _textures.find(texPath);
    if (it != _textures.end())
    {
        //テーブルに内にあったらロードするのではなくマップ内の
        //リソースを返す
        return _textures[texPath].get();
    }
    else
    {
        auto texture = CreateTexture(texPath);
        _textures[texPath] = texture;
        return texture.get();
    }
}
