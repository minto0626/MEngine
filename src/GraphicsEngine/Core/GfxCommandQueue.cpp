#include "GfxCommandQueue.h"

namespace Graphics
{
	bool GfxCommandQueue::Initialize(GfxDevice* device, D3D12_COMMAND_LIST_TYPE type)
	{
		_type = type;

		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; // タイムアウト無し
		desc.NodeMask = 0;  // アダプター１つ
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Type = type;

		auto result = device->Get()->CreateCommandQueue(
			&desc,
			IID_PPV_ARGS(_commandQueue.ReleaseAndGetAddressOf()));

		_commandQueue->SetName(L"command_queue");

		return SUCCEEDED(result);
	}

	void GfxCommandQueue::Execute(UINT numCommandLists, ID3D12CommandList* commandList)
	{
		_commandQueue->ExecuteCommandLists(numCommandLists, &commandList);
	}

	void GfxCommandQueue::Signal(GfxFence& fence)
	{
		fence.Signal(_commandQueue.Get());
	}
}