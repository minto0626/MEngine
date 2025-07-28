#include "GfxSwapChain.h"

namespace Graphics
{
	bool GfxSwapChain::Initialize(
        HWND hwnd,
        SIZE& size,
        GfxCommandQueue* commandQueue,
        IDXGIFactory6* factory)
	{
        DXGI_SWAP_CHAIN_DESC1 desc = {};
        desc.Width = size.cx;
        desc.Height = size.cy;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.Stereo = false;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.BufferCount = FRAME_BUFFER_COUNT;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.Scaling = DXGI_SCALING_STRETCH;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        HRESULT ret = factory->CreateSwapChainForHwnd(
            commandQueue->Get(),
            hwnd,
            &desc,
            nullptr,
            nullptr,
            (IDXGISwapChain1**)_swapChain.ReleaseAndGetAddressOf());
        if (FAILED(ret))
        {
            return false;
        }

        return true;
    }

	void GfxSwapChain::Present()
	{
        _swapChain->Present(1, 0);
	}
}