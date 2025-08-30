#include "RenderTarget.h"

#include <cassert>

namespace Graphics
{
	RenderTarget::~RenderTarget()
	{
		if (_descHandle.IsValid() && _ownerHeap != nullptr)
		{
			_ownerHeap->Free(_descHandle);
			_descHandle = {};
		}
	}

	void RenderTarget::Init(GfxDevice* device, UINT width, UINT height, DXGI_FORMAT format, DescriptorHeap& descHeap)
	{
		// todo
	}

	void RenderTarget::InitFromSwapChain(GfxDevice* device, GfxSwapChain* swapChain, DescriptorHeap& descHeap, UINT bufferIndex)
	{
		auto result = swapChain->Get()->GetBuffer(bufferIndex, IID_PPV_ARGS(_renderTarget.ReleaseAndGetAddressOf()));
		if (FAILED(result))
		{
			assert(0);
			return;
		}
		_renderTarget->SetName(L"render_target");

		_ownerHeap = &descHeap;
		_descHandle = descHeap.Allocate();
		device->Get()->CreateRenderTargetView(_renderTarget.Get(), nullptr, _descHandle.cpuHandle);
	}
}