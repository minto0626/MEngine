#pragma once
#include <d3d12.h>
#include <wrl.h>

#include "DescriptorHeap/DescriptorHeap.h"

class Texture
{
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> _texture;
    DescriptorHeap* _descHeap;
	DescriptorHandle _descHandle;

public:
    Texture() = default;
    ~Texture();

	void Init(ID3D12Device* device, DescriptorHeap* descHeap, Microsoft::WRL::ComPtr<ID3D12Resource> texture, DXGI_FORMAT format);

    void Release();

    DescriptorHandle GetSRV() const { return _descHandle; }

};
