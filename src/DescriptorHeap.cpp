#include "DescriptorHeap.h"
#include <cassert>

DescriptorHeap::DescriptorHeap(
	ID3D12Device* device,
	D3D12_DESCRIPTOR_HEAP_TYPE type,
	UINT numDescriptors,
	bool shaderVisible)
	: _type(type),
	_capacity(numDescriptors),
	_shaderVisible(shaderVisible),
	_descriptorSize(0)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = numDescriptors;
	desc.Type = type;
	desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 0;	// GPUは一つ前提

	if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(_heap.ReleaseAndGetAddressOf()))))
	{
		assert(0 && "ディスクリプタヒープの作成に失敗");
		return;
	}

	_descriptorSize = device->GetDescriptorHandleIncrementSize(type);

	for (UINT i = 0; i < numDescriptors; i++)
	{
		_freeIndices.push(i);
	}
}

DescriptorHandle DescriptorHeap::Allocate()
{
	if (_freeIndices.empty())
	{
		assert(0 && "ディスクリプタの空きスロットが無い");
	}

	UINT index = _freeIndices.front();
	_freeIndices.pop();

	DescriptorHandle handle = {};
	handle.cpuHandle = _heap->GetCPUDescriptorHandleForHeapStart();
	handle.cpuHandle.ptr += index * _descriptorSize;

	if (_shaderVisible)
	{
		handle.gpuHandle = _heap->GetGPUDescriptorHandleForHeapStart();
		handle.gpuHandle.ptr += index * _descriptorSize;
	}
	else
	{
		handle.gpuHandle.ptr = 0;
	}

	return handle;
}

std::vector<DescriptorHandle> DescriptorHeap::AllocateRange(UINT count)
{
	if (_freeIndices.size() < count)
	{
		assert(0 && "ディスクリプタの空きが足りない");
	}

	// 連続領域確保は、簡易的に first N available を使う（高速化の余地あり）
	std::vector<UINT> indecis;
	for (UINT i = 0; i < count; i++)
	{
		indecis.push_back(_freeIndices.front());
		_freeIndices.pop();
	}

	std::vector<DescriptorHandle> handles;
	for (UINT i = 0; i < count; i++)
	{
		DescriptorHandle handle;
		handle.cpuHandle = _heap->GetCPUDescriptorHandleForHeapStart();
		handle.cpuHandle.ptr += indecis[i] * _descriptorSize;

		if (_shaderVisible)
		{
			handle.gpuHandle = _heap->GetGPUDescriptorHandleForHeapStart();
			handle.gpuHandle.ptr += indecis[i] * _descriptorSize;
		}
		else
		{
			handle.gpuHandle.ptr = 0;
		}

		handles.push_back(handle);
	}

	return handles;
}

void DescriptorHeap::Free(const DescriptorHandle& handle)
{
	SIZE_T base = _heap->GetCPUDescriptorHandleForHeapStart().ptr;
	SIZE_T offset = handle.cpuHandle.ptr - base;

	if (offset % _descriptorSize != 0)
	{
		assert(0 && "無効なディスクリプタハンドルのアライメント");
		return;
	}

	UINT index = static_cast<UINT>(offset / _descriptorSize);
	_freeIndices.push(index);
}

void DescriptorHeap::Reset()
{
	// 全スロットを使える状態に戻す
	std::queue<UINT> empty;
	std::swap(_freeIndices, empty);

	for (UINT i = 0; i < _capacity; i++)
	{
		_freeIndices.push(i);
	}
}

ID3D12DescriptorHeap* DescriptorHeap::GetHeap() const
{
	return _heap.Get();
}