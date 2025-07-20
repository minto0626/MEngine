#pragma once
#include <d3d12.h>
#include <wrl.h>

#include "GfxDevice.h"

namespace Graphics
{
	class GfxFence
	{
	private:
		Microsoft::WRL::ComPtr<ID3D12Fence> _fence;
		HANDLE _event = nullptr;
		UINT64 _value = 0;

	public:
		~GfxFence();

		bool Initialize(GfxDevice* device);
		void Signal(ID3D12CommandQueue* commandQueue);
		void Wait();
	};
}