#pragma once
#include <d3d12.h>
#include <wrl.h>

#include "GfxDevice.h"
#include "GfxFence.h"

namespace Graphics
{
	class GfxCommandQueue
	{
	private:
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> _commandQueue;
		D3D12_COMMAND_LIST_TYPE _type;

	public:
		bool Initialize(GfxDevice* device, D3D12_COMMAND_LIST_TYPE type);
		void Execute(UINT numCommandLists, ID3D12CommandList* commandList);
		void Signal(GfxFence& fence);
		ID3D12CommandQueue* Get() const { return _commandQueue.Get(); }
		D3D12_COMMAND_LIST_TYPE GetType() const { return _type; }

	};
}