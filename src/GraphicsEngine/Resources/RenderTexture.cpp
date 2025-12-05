#include "RenderTexture.h"
#include "Core/d3dx12.h"
#include "Core/GfxCommandContext.h"
#include <cassert>

using namespace Microsoft::WRL;

namespace Graphics
{
	RenderTexture::~RenderTexture()
	{
		Release();
	}

	bool RenderTexture::Init(GfxDevice* device, DescriptorHeap* rtvHeap, DescriptorHeap* cbvSrvHeap, UINT width, UINT height, DXGI_FORMAT format)
	{
		if (!device || !rtvHeap || !cbvSrvHeap || width == 0 || height == 0) return false;

		_width = width;
		_height = height;
		_format = format;

		auto d3dDevice = device->Get();

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
			IID_PPV_ARGS(_resource.ReleaseAndGetAddressOf()));
		if (FAILED(result))
		{
			assert(0 && "レンダーテクスチャのリソース生成に失敗！");
			return false;
		}
		_resource->SetName(L"render_texture");

		// RTV 作成
		_rtvHeap = rtvHeap;
		_rtvHandle = _rtvHeap->Allocate();
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = format;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		d3dDevice->CreateRenderTargetView(_resource.Get(), &rtvDesc, _rtvHandle.cpuHandle);

		// SRV 作成
		_cbvSrvHeap = cbvSrvHeap;
		_srvHandle = _cbvSrvHeap->Allocate();
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		d3dDevice->CreateShaderResourceView(_resource.Get(), &srvDesc, _srvHandle.cpuHandle);

		return true;
	}

	void RenderTexture::Release()
	{
		// Descriptor を解放
		if (_rtvHeap != nullptr && _rtvHandle.IsValid())
		{
			_rtvHeap->Free(_rtvHandle);
			_rtvHandle = {};
		}
		if (_cbvSrvHeap != nullptr && _srvHandle.IsValid())
		{
			_cbvSrvHeap->Free(_srvHandle);
			_srvHandle = {};
		}
		_resource.Reset();
		_rtvHeap = nullptr;
		_cbvSrvHeap = nullptr;
		_width = _height = 0;
		_format = DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	bool RenderTexture::Resize(GfxDevice* device, DescriptorHeap* rtvHeap, DescriptorHeap* cbvSrvHeap, UINT width, UINT height, DXGI_FORMAT format)
	{
		Release();
		return Init(device, rtvHeap, cbvSrvHeap, width, height, format);
	}
}
