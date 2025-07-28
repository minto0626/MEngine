#include "GfxDevice.h"

using namespace Microsoft::WRL;

namespace Graphics
{
	bool GfxDevice::Initialize(IDXGIFactory6* factory)
	{
		// ビデオメモリの量が一番多いアダプターを探す
		ComPtr<IDXGIAdapter> adapterTmp;
		ComPtr<IDXGIAdapter> useAdapter;
		SIZE_T maxVideoMemorySize = 0;

		for (int i = 0; factory->EnumAdapters(i, &adapterTmp) != DXGI_ERROR_NOT_FOUND; i++)
		{
			DXGI_ADAPTER_DESC desc = {};
			adapterTmp->GetDesc(&desc);

			if (desc.DedicatedVideoMemory > maxVideoMemorySize)
			{
				useAdapter = adapterTmp;
				maxVideoMemorySize = desc.DedicatedVideoMemory;
			}
		}

		// フィーチャーレベルの選定
		D3D_FEATURE_LEVEL levels[] =
		{
			D3D_FEATURE_LEVEL_12_2,
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
		};

		for (auto level : levels)
		{
			if (SUCCEEDED(D3D12CreateDevice(
				useAdapter.Get(),
				level,
				IID_PPV_ARGS(_device.ReleaseAndGetAddressOf()))))
			{
				break;
			}
		}

		if (_device.Get() == nullptr)
		{
			return false;
		}

		_device->SetName(L"main_device");

		return true;
	}
}