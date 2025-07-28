#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace Graphics
{
	class GfxDevice
	{
	private:
		Microsoft::WRL::ComPtr<ID3D12Device> _device;

	public:
		bool Initialize(IDXGIFactory6* factory);
		ID3D12Device* Get() const { return _device.Get(); }

	};
}