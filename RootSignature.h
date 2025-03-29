#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>

using namespace Microsoft::WRL;

class RootSignature
{
private:
	ComPtr<ID3D12RootSignature> _rootSignature;
	std::vector<D3D12_DESCRIPTOR_RANGE> _ranges;
	std::vector<D3D12_ROOT_PARAMETER> _parameters;
	std::vector<D3D12_STATIC_SAMPLER_DESC> _samplers;

public:
	RootSignature();

	void AddDescriptorTable(UINT numDescriptors, UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility, D3D12_DESCRIPTOR_RANGE_TYPE rangeType);
	void AddStaticSampler(UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility);
	void Build(ID3D12Device* device);

	ID3D12RootSignature* Get() const { return _rootSignature.Get(); }
};