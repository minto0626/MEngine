#include "Texture.h"
#include <cassert>

void Texture::Init(ID3D12Device* device, ComPtr<ID3D12Resource> texture)
{
	_texture = texture;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;	// GPUは一つ前提
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	auto result = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(_descHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		assert(0, "ヒープの作成に失敗!");
		return;
	}

	auto handle = _descHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = _texture->GetDesc().Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;	// ミップマップ使用しない
	device->CreateShaderResourceView(
		_texture.Get(),
		&srvDesc,
		handle);
}