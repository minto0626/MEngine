#include "ConstantBuffer.h"
#include "d3dx12.h"
#include <cassert>

ConstantBuffer::~ConstantBuffer()
{
	if (_buffer != nullptr)
	{
		_buffer->Unmap(0, nullptr);
	}
}

void ConstantBuffer::Init(ID3D12Device* device, UINT size)
{
	_bufferSize = (size + 0xff) & ~0xff;	// サイズを256アライメント

	auto heapDesc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(_bufferSize);
	auto result = device->CreateCommittedResource(
		&heapDesc,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_buffer.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		assert(0);
		return;
	}

	_mappedData = nullptr;
	result = _buffer->Map(0, nullptr, &_mappedData);
	if (FAILED(result))
	{
		assert(0);
		return;
	}

	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 1;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	result = device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(_descHeap.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		assert(0);
		return;
	}

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _buffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = _buffer->GetDesc().Width;
	device->CreateConstantBufferView(&cbvDesc, _descHeap->GetCPUDescriptorHandleForHeapStart());
}

void ConstantBuffer::Update(const void* data, UINT size)
{
	if (size > _bufferSize)
	{
		assert(0);
		return;
	}

	memcpy(_mappedData, data, size);
}