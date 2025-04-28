#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <vector>

// ポリゴンの表示テスト
#include "TextureLoader.h"
#include "Sprite.h"
#include "SpriteRenderer.h"
#include "Camera.h"
//

#include "InputSystem.h"
#include "GameTime.h"

class MEngine
{
private:

	HWND _hwnd;

	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	ComPtr<ID3D12Device> _device = nullptr;
	ComPtr<ID3D12CommandAllocator> _commandAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> _commandList = nullptr;
	ComPtr<ID3D12CommandQueue> _commandQueue = nullptr;
	UINT _rtvDescriptorSize;

	ComPtr<IDXGIFactory6> _dxgiFactory = nullptr;
	ComPtr<IDXGISwapChain4> _swapchain = nullptr;
	const unsigned int FRAME_BUFFER_COUNT = 2;
	ComPtr<ID3D12DescriptorHeap> _rtvHeap = nullptr;
	std::vector<ComPtr<ID3D12Resource>> _renderTargets;

	ComPtr<ID3D12Fence> _fence = nullptr;
	UINT64 _fenceValue = 0;
	HANDLE _fenceEvent = nullptr;

	D3D12_RESOURCE_STATES _renderTargetStates[2] = { D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COMMON };

	IDXGIFactory6* CreateDXGIFactory();
	bool CreateDevice(IDXGIFactory6* dxgiFactory);
	bool CreateCommandList();
	bool CreateCommandQueue();
	bool CreateSwapchain(HWND hwnd, SIZE& windowSize, IDXGIFactory6* dxgiFactory);
	bool CreateFinalRenderTarget();
	bool CreateSynchronizationWithGPUObject();

	void BegineRender();
	void EndRender();
	void PresentFrame();
	void WaitDraw();

	void ResourceBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to);

	// ★ポリゴンの表示テスト★
	TextureLoader texLoader;
	Sprite sprite;
	SpriteRenderer spriteRenderer;
	Camera camera2D;
	//

	InputSystem input;
	Engine::Time time;

public:
	~MEngine();

	bool Init(HWND hwnd, HINSTANCE hInstance, SIZE& windowSize);
	void Update();
	void Draw();

	void ToggleFullScreen();
};