#pragma once
#include <d3d12.h>
#include <wrl.h>

using namespace Microsoft::WRL;

class IndexBuffer
{
private:
	ComPtr<ID3D12Resource> _indexBuffer;
	ComPtr<ID3D12Resource> _uploadBuffer;
	D3D12_INDEX_BUFFER_VIEW _indexBufferView;
	UINT _indexNum;
	DXGI_FORMAT _format;

public:
	void Init(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* commndList,
		const void* indexData,
		UINT indexNum,
		DXGI_FORMAT format);
};