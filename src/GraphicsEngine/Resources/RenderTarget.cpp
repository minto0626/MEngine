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
        if (_colorTextureHandle.IsValid() && _cbvSrvHeap != nullptr)
        {
            _cbvSrvHeap->Free(_colorTextureHandle);
            _colorTextureHandle = {};
        }
        if (_depthTextureHandle.IsValid() && _cbvSrvHeap != nullptr)
        {
            _cbvSrvHeap->Free(_depthTextureHandle);
            _depthTextureHandle = {};
        }
        _colorBuffer.Reset();
        _depthBuffer.Reset();
    }

    bool RenderTarget::InitColor(GfxDevice& device, UINT width, UINT height, DXGI_FORMAT format, DescriptorHeap& rtvHeap, DescriptorHeap* cbvSrvHeap)
    {
        if (width == 0 || height == 0) return false;

        _width = width;
        _height = height;

        auto d3dDevice = device.Get();

        auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto texDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, 1, 1);
        texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        CD3DX12_CLEAR_VALUE clearValue(format, clearColor);

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
            _cbvSrvHeap = cbvSrvHeap;
            _colorTextureHandle = _cbvSrvHeap->Allocate();
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = format;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = 1;
            d3dDevice->CreateShaderResourceView(_colorBuffer.Get(), &srvDesc, _colorTextureHandle.cpuHandle);
        }

        return true;
    }

    bool RenderTarget::InitDepth(GfxDevice& device, UINT width, UINT height, DXGI_FORMAT resourceFormat, DXGI_FORMAT dsvFormat, DescriptorHeap& dsvHeap, DXGI_FORMAT srvFormat, DescriptorHeap* cbvSrvHeap)
    {
        if (width == 0 || height == 0) return false;

        _width = width;
        _height = height;

        auto d3dDevice = device.Get();

        auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto texDesc = CD3DX12_RESOURCE_DESC::Tex2D(resourceFormat, width, height, 1, 1);
        texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        CD3DX12_CLEAR_VALUE clearValue(dsvFormat, 1.0f, 0);

        auto result = d3dDevice->CreateCommittedResource(
            &heapProp,
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &clearValue,
            IID_PPV_ARGS(_depthBuffer.ReleaseAndGetAddressOf()));
        if (FAILED(result))
        {
            assert(0 && "レンダーテクスチャのリソース生成に失敗！");
            return false;
        }
        _depthBuffer->SetName(L"render_target_depth");

        _dsvHeap = &dsvHeap;
        _dsvHandle = _dsvHeap->Allocate();
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = dsvFormat;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        d3dDevice->CreateDepthStencilView(_depthBuffer.Get(), &dsvDesc, _dsvHandle.cpuHandle);

        // SRV 作成
        if (cbvSrvHeap != nullptr)
        {
            _cbvSrvHeap = cbvSrvHeap;
            _depthTextureHandle = _cbvSrvHeap->Allocate();
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = srvFormat;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = 1;
            d3dDevice->CreateShaderResourceView(_depthBuffer.Get(), &srvDesc, _depthTextureHandle.cpuHandle);
        }

        return true;
    }

	bool RenderTarget::InitFromSwapChain(GfxDevice* device, GfxSwapChain* swapChain, DescriptorHeap& rtvHeap, UINT bufferIndex)
	{
		auto result = swapChain->Get()->GetBuffer(bufferIndex, IID_PPV_ARGS(_colorBuffer.ReleaseAndGetAddressOf()));
		if (FAILED(result))
		{
			assert(0);
			return false;
		}
		_colorBuffer->SetName(L"render_target");

        _width = static_cast<UINT>(_colorBuffer->GetDesc().Width);
        _height = _colorBuffer->GetDesc().Height;

		_rtvHeap = &rtvHeap;
		_rtvHandle = rtvHeap.Allocate();
		device->Get()->CreateRenderTargetView(_colorBuffer.Get(), nullptr, _rtvHandle.cpuHandle);

        return true;
	}
}
