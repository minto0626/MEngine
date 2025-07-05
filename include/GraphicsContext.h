#pragma once
#include <d3d12.h>

class GraphicsContext
{
public:
	ID3D12Device* device = nullptr;
	ID3D12GraphicsCommandList* commandList = nullptr;
	ID3D12CommandQueue* commandQueue = nullptr;
	ID3D12CommandAllocator* commandAllocator = nullptr;
	ID3D12Fence* fence = nullptr;
	UINT64* fenceValue = nullptr;
	HANDLE fenceEvent = nullptr;

	void ExecuteCommand();
	void WaitDraw();
	void ResetCommand();
};