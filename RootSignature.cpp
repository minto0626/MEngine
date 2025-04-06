#include "RootSignature.h"
#include "d3dx12.h"
#include <cassert>
#include "Debug.h"

RootSignature::RootSignature()
{
	_ranges.clear();
	_parameters.clear();
	_samplers.clear();
}

void RootSignature::AddDescriptorTable(UINT numDescriptors, UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility, D3D12_DESCRIPTOR_RANGE_TYPE rangeType)
{
	CD3DX12_DESCRIPTOR_RANGE range = {};
	range.Init(rangeType, numDescriptors, shaderRegister);
	_ranges.push_back(range);

	CD3DX12_ROOT_PARAMETER param = {};
	param.InitAsDescriptorTable(1, nullptr, visibility);	// Bulid時にレンジを設定する

	_parameters.push_back(param);
}

void RootSignature::AddStaticSampler(UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility)
{
	CD3DX12_STATIC_SAMPLER_DESC desc = {};
	desc.Init(shaderRegister);
	desc.ShaderVisibility = visibility;

	_samplers.push_back(desc);
}

void RootSignature::Build(ID3D12Device* device)
{
	if (!_ranges.empty())
	{
		size_t index = 0;
		for (auto& param : _parameters)
		{
			if (param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
			{
				param.DescriptorTable.pDescriptorRanges = &_ranges[index++];
			}
		}
	}

	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	desc.pParameters = _parameters.empty() ? nullptr : _parameters.data();
	desc.NumParameters = static_cast<UINT>(_parameters.size());
	desc.pStaticSamplers = _samplers.empty() ? nullptr : _samplers.data();
	desc.NumStaticSamplers = static_cast<UINT>(_samplers.size());

	ComPtr<ID3DBlob> errorBlob = nullptr;
	ComPtr<ID3DBlob> rootSignatureBlob = nullptr;
	auto result = D3D12SerializeRootSignature(
		&desc,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSignatureBlob,
		&errorBlob);
	if (FAILED(result))
	{
		if (errorBlob != nullptr)
		{
			std::string error(static_cast<char*>(errorBlob->GetBufferPointer()), errorBlob->GetBufferSize());
			Debug::ShowErrorMessageBox(error, "ルートシグネチャのシリアライズに失敗!");
		}
		assert(0);
		return;
	}

	result = device->CreateRootSignature(
		0,	// GPUは１つ前提
		rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(_rootSignature.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		Debug::ShowErrorMessageBox("ルートシグネチャの生成に失敗!", "エラー");
		return;
	}
}