#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "DescriptorHeap.h"

class Texture
{
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> _texture;
	DescriptorHandle _descHandle;

public:
	void Init(ID3D12Device* device, DescriptorHeap* descHeap, Microsoft::WRL::ComPtr<ID3D12Resource> texture);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return _descHandle.cpuHandle; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return _descHandle.gpuHandle; }
};