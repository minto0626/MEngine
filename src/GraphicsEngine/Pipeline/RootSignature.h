#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <unordered_map>
#include <string>

class RootSignature
{
private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> _rootSignature;
	std::vector<D3D12_DESCRIPTOR_RANGE> _ranges;
	std::vector<D3D12_ROOT_PARAMETER> _parameters;
	std::vector<D3D12_STATIC_SAMPLER_DESC> _samplers;
	std::unordered_map<std::string, UINT> _rootIndexMap;

public:
	RootSignature();

	void AddDescriptorTable(const std::string& name, UINT numDescriptors, UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility, D3D12_DESCRIPTOR_RANGE_TYPE rangeType);
	void AddStaticSampler(UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility);
	void Build(ID3D12Device* device);

	ID3D12RootSignature* Get() const { return _rootSignature.Get(); }
	UINT GetRootIndex(const std::string& name) const;
};