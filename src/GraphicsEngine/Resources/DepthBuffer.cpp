#include "DepthBuffer.h"

#include "Core//d3dx12.h"

namespace Graphics
{
	DepthBuffer::~DepthBuffer()
	{
		if (_descHandle.IsValid() && _ownerHeap != nullptr)
		{
			_ownerHeap->Free(_descHandle);
			_descHandle = {};
		}
	}

	void DepthBuffer::Init(GfxDevice* device, UINT width, UINT height, DXGI_FORMAT format, DescriptorHeap& descHeap)
	{
		auto depthHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto depthResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			format,
			width, height,
			1,	// テクスチャ配列でも、3Dテクスチャでもない
			1,	// ミップマップしないので1
			1,	// サンプルは1ピクセルあたり1つ
			0,	// クオリティは最低
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
		);
		CD3DX12_CLEAR_VALUE depthClearValue(
			format,	// 32bit floatでクリア
			1.0f,	// 1.0fでクリア
			0);		// ステンシルは使わない
		auto result = device->Get()->CreateCommittedResource(
			&depthHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&depthResDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthClearValue,
			IID_PPV_ARGS(_buffer.ReleaseAndGetAddressOf()));
		if (FAILED(result))
		{
			return;
		}
		_buffer->SetName(L"depth_buffer");

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = format;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;	// 2Dテクスチャ
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		_descHandle = descHeap.Allocate();
		device->Get()->CreateDepthStencilView(_buffer.Get(), &dsvDesc, _descHandle.cpuHandle);
	}
}