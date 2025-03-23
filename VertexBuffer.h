#pragma once
#include <d3d12.h>
#include <wrl.h>

using namespace Microsoft::WRL;

class VertexBuffer
{
private:
	ComPtr<ID3D12Resource> _vertexBuffer;
	ComPtr<ID3D12Resource> _uploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW _vertexBufferView;
	UINT _vertexNum;
	UINT _vertexStride;

public:
	void Init(ID3D12Device* device,
		ID3D12GraphicsCommandList* commndList,
		const void* vertexData,
		UINT vertexNum,
		UINT vertexStride);
};