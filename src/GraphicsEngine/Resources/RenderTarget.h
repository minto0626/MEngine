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
		Microsoft::WRL::ComPtr<ID3D12Resource> _colorBuffer;
        Microsoft::WRL::ComPtr<ID3D12Resource> _depthBuffer;
		DescriptorHeap* _rtvHeap;
        DescriptorHeap* _dsvHeap;
        DescriptorHeap* _cbvSrvHeap;
		DescriptorHandle _rtvHandle;
        DescriptorHandle _dsvHandle;
        DescriptorHandle _colorTextureHandle;
        DescriptorHandle _depthTextureHandle;
        UINT _width;
        UINT _height;

	public:
        RenderTarget() = default;
		~RenderTarget();

		bool InitColor(GfxDevice& device, UINT width, UINT height, DXGI_FORMAT format, DescriptorHeap& rtvHeap, DescriptorHeap* cbvSrvHeap = nullptr);
        bool InitDepth(GfxDevice& device, UINT width, UINT height, DXGI_FORMAT resourceFormat, DXGI_FORMAT dsvFormat, DescriptorHeap& dsvHeap, DXGI_FORMAT srvFormat = DXGI_FORMAT_UNKNOWN, DescriptorHeap* cbvSrvHeap = nullptr);
		bool InitFromSwapChain(GfxDevice* device, GfxSwapChain* swapChain, DescriptorHeap& rtvHeap, UINT bufferIndex);

        void Release();

		ID3D12Resource* GetColorBuffer() const { return _colorBuffer.Get(); }
        ID3D12Resource* GetDepthBuffer() const { return _depthBuffer.Get(); }
        DescriptorHandle GetRTV() const { return _rtvHandle; }
        DescriptorHandle GetDSV() const { return _dsvHandle; }
        DescriptorHandle GetColorSRV() const { return _colorTextureHandle; }
        DescriptorHandle GetDepthSRV() const { return _depthTextureHandle; }

        UINT GetWidth() const { return _width; }
        UINT GetHeight() const { return _height; }

	};
}
