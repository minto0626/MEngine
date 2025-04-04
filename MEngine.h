#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <vector>

// ポリゴンの表示テスト
#include "Vector.h"
#include "Transform.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "TextureLoader.h"
#include "Texture.h"
#include "ConstantBuffer.h"
#include "InputLayoutHelper.h"
#include "PipelineState.h"
#include "RootSignature.h"
//

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
	Transform transform;
	VertexBuffer vertexBuffer;
	IndexBuffer indexBuffer;
	Shader vertexShader;
	Shader pixelShader;
	TextureLoader texLoader;
	Texture texBuffer;
	ConstantBuffer transformBuffer;
	PipelineState pipelineState;
	RootSignature rootSignature;

public:
	~MEngine();

	bool Init(HWND hwnd, SIZE& windowSize);
	void Update();
	void Draw();

	void ToggleFullScreen();
};