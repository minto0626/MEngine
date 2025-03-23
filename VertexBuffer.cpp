#include "VertexBuffer.h"
#include "d3dx12.h"
#include <cassert>

void VertexBuffer::Init(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* commndList,
	const void* vertexData,
	UINT vertexNum,
	UINT vertexStride)
{
	_vertexNum = vertexNum;
	_vertexStride = vertexStride;

	UINT bufferSize = vertexNum * vertexStride;

	// デフォルトのヒープに頂点バッファーを作成
	auto def_heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
	auto result = device->CreateCommittedResource(
		&def_heapProp,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(_vertexBuffer.ReleaseAndGetAddressOf()));
	if (FAILED(result)) { assert("頂点バッファーの作成に失敗!"); return; }

	// アップロード用のヒープに中間バッファーを作成
	auto upload_heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	result = device->CreateCommittedResource(
		&upload_heapProp,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_uploadBuffer.ReleaseAndGetAddressOf()));
	if (FAILED(result)) { assert("アップロード用のバッファーの作成に失敗!"); return; }

	// CPUからアップロード用バッファーにデータをコピー
	void* mappedData = nullptr;
	result = _uploadBuffer->Map(0, nullptr, &mappedData);
	if (FAILED(result)) { assert("アップロード用のバッファのマップに失敗"); return; }
	memcpy(mappedData, vertexData, bufferSize);
	_uploadBuffer->Unmap(0, nullptr);

	// アップロード用のバッファーからデフォルト用のバッファーにデータを転送
	commndList->CopyResource(_vertexBuffer.Get(), _uploadBuffer.Get());

	// リソース状態を頂点バッファー用に遷移
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		_vertexBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	commndList->ResourceBarrier(1, &barrier);

	_vertexBuffer->SetName(L"VertexBuffer");

	// ビューを設定
	_vertexBufferView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
	_vertexBufferView.SizeInBytes = bufferSize;
	_vertexBufferView.StrideInBytes = vertexStride;
}