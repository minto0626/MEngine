#pragma once
#include <d3d12.h>
#include <wrl.h>

#include "DescriptorHeap/DescriptorHeap.h"

class ConstantBuffer
{
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> _buffer;
	UINT _bufferSize;
	void* _mappedData;
    DescriptorHeap* _descHeap;
	DescriptorHandle _descHandle;

public:
	~ConstantBuffer();
	void Init(ID3D12Device* device, DescriptorHeap* descHeap, UINT size);
	void Update(const void* data, UINT size);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return _descHandle.cpuHandle; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return _descHandle.gpuHandle; }
};
