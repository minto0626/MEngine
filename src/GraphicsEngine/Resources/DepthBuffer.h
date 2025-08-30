#pragma once
#include <d3d12.h>
#include <wrl.h>

#include "Core/GfxDevice.h"
#include "DescriptorHeap/DescriptorHeap.h"

namespace Graphics
{
	class DepthBuffer
	{
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> _buffer;
		DescriptorHandle _descHandle;
		DescriptorHeap* _ownerHeap;

	public:
		~DepthBuffer();

		void Init(GfxDevice* device, UINT width, UINT height, DXGI_FORMAT format, DescriptorHeap& descHeap);

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return _descHandle.cpuHandle; }
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return _descHandle.gpuHandle; }

	};
}