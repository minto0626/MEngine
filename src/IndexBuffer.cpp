#include "IndexBuffer.h"
#include "d3dx12.h"
#include <cassert>

void IndexBuffer::Init(
	ID3D12Device* device,
	Graphics::GfxCommandContext& commandContext,
	const void* indexData,
	UINT indexNum,
	DXGI_FORMAT format)
{
	_indexNum = indexNum;
	_format = format;

	UINT bufferSize = indexNum * (format == DXGI_FORMAT_R16_UINT ? 2 : 4);

	auto def_heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
	auto result = device->CreateCommittedResource(
		&def_heapProp,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(_indexBuffer.ReleaseAndGetAddressOf()));
	if (FAILED(result)) { assert("インデックスバッファーの作成に失敗!"); return; }

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
	memcpy(mappedData, indexData, bufferSize);
	_uploadBuffer->Unmap(0, nullptr);

	commandContext.ResourceBarrier(
		_indexBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_COPY_DEST);

	// アップロード用のバッファーからデフォルト用のバッファーにデータを転送
	commandContext.CopyResource(_indexBuffer.Get(), _uploadBuffer.Get());

	// リソース状態を頂点バッファー用に遷移
	commandContext.ResourceBarrier(
		_indexBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_INDEX_BUFFER);

	_indexBuffer->SetName(L"index_buffer");

	// ビューを設定
	_indexBufferView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
	_indexBufferView.SizeInBytes = bufferSize;
	_indexBufferView.Format = format;
}