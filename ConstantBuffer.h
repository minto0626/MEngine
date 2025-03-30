#pragma once
#include <d3d12.h>
#include <wrl.h>

using namespace Microsoft::WRL;

class ConstantBuffer
{
private:
	ComPtr<ID3D12Resource> _buffer;
	UINT _bufferSize;
	void* _mappedData;
	ComPtr<ID3D12DescriptorHeap> _descHeap;

public:
	~ConstantBuffer();
	void Init(ID3D12Device* device, UINT size);
	void Update(const void* data, UINT size);

	ID3D12DescriptorHeap* GetHeap() const { return _descHeap.Get(); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return _descHeap->GetCPUDescriptorHandleForHeapStart(); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return _descHeap->GetGPUDescriptorHandleForHeapStart(); }
};