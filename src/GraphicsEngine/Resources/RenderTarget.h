#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <memory>

#include "Core/GfxDevice.h"
#include "Core/GfxSwapChain.h"
#include "DescriptorHeap/DescriptorHeap.h"
#include "Resources/Texture.h"
#include "Math/Color.h"

namespace Graphics
{
	class RenderTarget
	{
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> _colorBuffer;
        Microsoft::WRL::ComPtr<ID3D12Resource> _depthBuffer;
        Color* _rtvClearColor;
        float _dsvClearValue = 0;
		DescriptorHeap* _rtvHeap;
        DescriptorHeap* _dsvHeap;
		DescriptorHandle _rtvHandle;
        DescriptorHandle _dsvHandle;
        std::unique_ptr<Texture> _colorTexture;
        std::unique_ptr<Texture> _depthTexture;
        UINT _width;
        UINT _height;

	public:
        RenderTarget() = default;
		~RenderTarget();

		bool InitColor(GfxDevice& device, UINT width, UINT height, DXGI_FORMAT format, Color& clearColor, DescriptorHeap& rtvHeap, DescriptorHeap* cbvSrvHeap = nullptr);
        bool InitDepth(GfxDevice& device, UINT width, UINT height, DXGI_FORMAT resourceFormat, DXGI_FORMAT dsvFormat, float clearValue, DescriptorHeap& dsvHeap, DXGI_FORMAT srvFormat = DXGI_FORMAT_UNKNOWN, DescriptorHeap* cbvSrvHeap = nullptr);
		bool InitFromSwapChain(GfxDevice* device, GfxSwapChain* swapChain, DescriptorHeap& rtvHeap, Color& clearColor, UINT bufferIndex);

        void Release();

		ID3D12Resource* GetColorBuffer() const { return _colorBuffer.Get(); }
        ID3D12Resource* GetDepthBuffer() const { return _depthBuffer.Get(); }
        DescriptorHandle GetRTV() const { return _rtvHandle; }
        DescriptorHandle GetDSV() const { return _dsvHandle; }
        Color* GetRTVClearColor() const { return _rtvClearColor; }
        float GetDSVClearValue() const { return _dsvClearValue; }
        Texture* GetColorTexture() const { return _colorTexture.get(); }
        Texture* GetDepthTexture() const { return _depthTexture.get(); }

        UINT GetWidth() const { return _width; }
        UINT GetHeight() const { return _height; }

	};
}
