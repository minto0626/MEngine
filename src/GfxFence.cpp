#include "GfxFence.h"

namespace Graphics
{
	GfxFence::~GfxFence()
	{
		if (_event != nullptr)
		{
			CloseHandle(_event);
		}
	}

	bool GfxFence::Initialize(GfxDevice* device)
	{
		auto result = device->Get()->CreateFence(
			_value,
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(_fence.ReleaseAndGetAddressOf()));
		if (FAILED(result))
		{
			return false;
		}

		_fence->SetName(L"fence");

		// GPUと同期を取るときのイベントハンドルを作成
		_event = CreateEvent(nullptr, false, false, nullptr);
		if (_event == nullptr)
		{
			return false;
		}

		return true;
	}

	void GfxFence::Signal(ID3D12CommandQueue* commandQueue)
	{
		commandQueue->Signal(_fence.Get(), ++_value);
	}

	void GfxFence::Wait()
	{
		if (_fence->GetCompletedValue() != _value)
		{
			_fence->SetEventOnCompletion(_value, _event);
			WaitForSingleObject(_event, INFINITE);
		}
	}
}