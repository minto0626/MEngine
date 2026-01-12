#include "Texture.h"

#include <cassert>

using namespace Microsoft::WRL;

Texture::~Texture()
{
    Release();
}

void Texture::Release()
{
    if (_descHandle.IsValid() && _descHeap != nullptr)
    {
        _descHeap->Free(_descHandle);
        _descHandle = {};
    }

    if (_texture)
    {
        _texture.Reset();
    }
}

void Texture::Init(ID3D12Device* device, DescriptorHeap* descHeap, ComPtr<ID3D12Resource> texture, DXGI_FORMAT format)
{
	_texture = texture;
    _descHeap = descHeap;
	_descHandle = _descHeap->Allocate();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;	// ミップマップ使用しない
	device->CreateShaderResourceView(
		_texture.Get(),
		&srvDesc,
		_descHandle.cpuHandle);
}
