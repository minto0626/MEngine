#include "RenderTarget.h"
#include "Core/d3dx12.h"

#include <cassert>

namespace Graphics
{
	RenderTarget::~RenderTarget()
	{
        Release();
	}

    void RenderTarget::Release()
    {
        if (_rtvHandle.IsValid() && _rtvHeap != nullptr)
        {
            _rtvHeap->Free(_rtvHandle);
            _rtvHandle = {};
        }
        if (_dsvHandle.IsValid() && _dsvHeap != nullptr)
        {
            _dsvHeap->Free(_dsvHandle);
            _dsvHandle = {};
        }
        _colorBuffer.Reset();
        _depthBuffer.Reset();

        if (_colorTexture != nullptr)
        {
            _colorTexture->Release();
        }
        if (_depthTexture != nullptr)
        {
            _depthTexture->Release();
        }
    }

    bool RenderTarget::InitColor(GfxDevice& device, UINT width, UINT height, DXGI_FORMAT format, Color& clearColor, DescriptorHeap& rtvHeap, DescriptorHeap* cbvSrvHeap)
    {
        if (width == 0 || height == 0) return false;

        _width = width;
        _height = height;

        auto d3dDevice = device.Get();

        auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto texDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, 1, 1);
        texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        _rtvClearColor = &clearColor;
        const float cc[4]{ _rtvClearColor->r, _rtvClearColor->g, _rtvClearColor->b, _rtvClearColor->a };
        CD3DX12_CLEAR_VALUE clearValue(format, cc);

        auto result = d3dDevice->CreateCommittedResource(
            &heapProp,
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            &clearValue,
            IID_PPV_ARGS(_colorBuffer.ReleaseAndGetAddressOf()));
        if (FAILED(result))
        {
            assert(0 && "レンダーテクスチャのリソース生成に失敗！");
            return false;
        }
        _colorBuffer->SetName(L"render_target_color");

        // RTV 作成
        _rtvHeap = &rtvHeap;
        _rtvHandle = _rtvHeap->Allocate();
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = format;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        d3dDevice->CreateRenderTargetView(_colorBuffer.Get(), &rtvDesc, _rtvHandle.cpuHandle);

        // SRV 作成
        if (cbvSrvHeap != nullptr)
        {
            _colorTexture = std::make_unique<Texture>();
            _colorTexture->Init(d3dDevice, cbvSrvHeap, _colorBuffer, format);
        }

        return true;
    }

    bool RenderTarget::InitDepth(GfxDevice& device, UINT width, UINT height, DXGI_FORMAT resourceFormat, DXGI_FORMAT dsvFormat, float clearValue, DescriptorHeap& dsvHeap, DXGI_FORMAT srvFormat, DescriptorHeap* cbvSrvHeap)
    {
        if (width == 0 || height == 0) return false;

        _width = width;
        _height = height;

        auto d3dDevice = device.Get();

        auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto texDesc = CD3DX12_RESOURCE_DESC::Tex2D(resourceFormat, width, height, 1, 1);
        texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        _dsvClearValue = clearValue;
        CD3DX12_CLEAR_VALUE _clearValue(dsvFormat, _dsvClearValue, 0);

        auto result = d3dDevice->CreateCommittedResource(
            &heapProp,
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &_clearValue,
            IID_PPV_ARGS(_depthBuffer.ReleaseAndGetAddressOf()));
        if (FAILED(result))
        {
            assert(0 && "レンダーテクスチャのリソース生成に失敗！");
            return false;
        }
        _depthBuffer->SetName(L"render_target_depth");

        // DSV 作成
        _dsvHeap = &dsvHeap;
        _dsvHandle = _dsvHeap->Allocate();
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = dsvFormat;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        d3dDevice->CreateDepthStencilView(_depthBuffer.Get(), &dsvDesc, _dsvHandle.cpuHandle);

        // SRV 作成
        if (cbvSrvHeap != nullptr)
        {
            _depthTexture = std::make_unique<Texture>();
            _depthTexture->Init(d3dDevice, cbvSrvHeap, _depthBuffer, srvFormat);
        }

        return true;
    }

	bool RenderTarget::InitFromSwapChain(GfxDevice* device, GfxSwapChain* swapChain, DescriptorHeap& rtvHeap, Color& clearColor, UINT bufferIndex)
	{
		auto result = swapChain->Get()->GetBuffer(bufferIndex, IID_PPV_ARGS(_colorBuffer.ReleaseAndGetAddressOf()));
		if (FAILED(result))
		{
			assert(0);
			return false;
		}
		_colorBuffer->SetName(L"render_target");

        _rtvClearColor = &clearColor;

        _width = static_cast<UINT>(_colorBuffer->GetDesc().Width);
        _height = _colorBuffer->GetDesc().Height;

		_rtvHeap = &rtvHeap;
		_rtvHandle = rtvHeap.Allocate();
		device->Get()->CreateRenderTargetView(_colorBuffer.Get(), nullptr, _rtvHandle.cpuHandle);

        return true;
	}
}
