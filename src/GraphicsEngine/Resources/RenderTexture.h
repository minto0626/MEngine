#pragma once
#include <d3d12.h>
#include <wrl.h>

#include "Core/GfxDevice.h"
#include "DescriptorHeap/DescriptorHeap.h"

namespace Graphics
{
	class GfxCommandContext;

	class RenderTexture
	{
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> _resource;
		DescriptorHeap* _rtvHeap = nullptr;
		DescriptorHeap* _cbvSrvHeap = nullptr;
		DescriptorHandle _rtvHandle;
		DescriptorHandle _srvHandle;
		UINT _width = 0;
		UINT _height = 0;
		DXGI_FORMAT _format = DXGI_FORMAT_R8G8B8A8_UNORM;

	public:
		RenderTexture() = default;
		~RenderTexture();

		bool Init(
            GfxDevice* device,
            DescriptorHeap* rtvHeap,
            DescriptorHeap* cbvSrvHeap,
            UINT width,
            UINT height,
            DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);

		// 解放（Descriptor の解放を行う）
		void Release();

		ID3D12Resource* GetResource() const { return _resource.Get(); }
		DescriptorHandle GetRTV() const { return _rtvHandle; }
		DescriptorHandle GetSRV() const { return _srvHandle; }

		UINT GetWidth() const { return _width; }
		UINT GetHeight() const { return _height; }

		// リサイズ（内部で Release + Init）
		bool Resize(
            GfxDevice* device,
            DescriptorHeap* rtvHeap,
            DescriptorHeap* cbvSrvHeap,
            UINT width,
            UINT height,
            DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
	};
}
