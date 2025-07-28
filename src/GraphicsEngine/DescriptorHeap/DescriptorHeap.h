#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <queue>

// CPU/GPU ディスクリプタハンドルペア構造体
struct DescriptorHandle
{
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    bool IsValid() const { return cpuHandle.ptr != 0; }
};

class DescriptorHeap
{
public:
    DescriptorHeap(
    ID3D12Device* device,
        D3D12_DESCRIPTOR_HEAP_TYPE type,
        UINT numDescriptor,
        bool shaderVisible = true);

    // 単一ディスクリプタを確保
    DescriptorHandle Allocate();
    // 複数まとめて確保（連続領域）
    std::vector<DescriptorHandle> AllocateRange(UINT count);
    // １つを解放
    void Free(const DescriptorHandle& handle);
    // 全て開放
    void Reset();
    // ディスクリプタヒープ本体を取得
    ID3D12DescriptorHeap* GetHeap() const;

private:
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _heap;
    UINT _descriptorSize;
    UINT _capacity;
    D3D12_DESCRIPTOR_HEAP_TYPE _type;
    bool _shaderVisible;

    std::queue<UINT> _freeIndices;
};