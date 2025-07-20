#pragma once
#include <dxgi1_6.h>
#include <wrl.h>

#include "GfxCommandQueue.h"

namespace Graphics
{
	class GfxSwapChain
	{
	private:
		Microsoft::WRL::ComPtr<IDXGISwapChain4> _swapChain;
		const unsigned int FRAME_BUFFER_COUNT = 2;

	public:
		bool Initialize(HWND hwnd, SIZE& size, GfxCommandQueue* commandQueue, IDXGIFactory6* factory);
		void Present();
		IDXGISwapChain4* Get() const { return _swapChain.Get(); }
	};
}