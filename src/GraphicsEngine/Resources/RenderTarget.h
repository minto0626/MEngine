#pragma once
#include <d3d12.h>
#include <wrl.h>

#include "Core/GfxDevice.h"
#include "Core/GfxSwapChain.h"
#include "DescriptorHeap/DescriptorHeap.h"

namespace Graphics
{
	class RenderTarget
	{
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> _renderTarget;
		DescriptorHeap* _ownerHeap;
		DescriptorHandle _descHandle;

	public:
		~RenderTarget();

		void Init(GfxDevice* device, UINT width, UINT height, DXGI_FORMAT format, DescriptorHeap& descHeap);
		void InitFromSwapChain(GfxDevice* device, GfxSwapChain* swapChain, DescriptorHeap& descHeap, UINT bufferIndex);

		ID3D12Resource* GetResource() const { return _renderTarget.Get(); }
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return _descHandle.cpuHandle; }
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return _descHandle.gpuHandle; }

	};
}