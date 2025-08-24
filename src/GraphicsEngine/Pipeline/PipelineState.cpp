#include "PipelineState.h"
#include "Shader/Shader.h"
#include "RootSignature.h"
#include "InputLayoutHelper.h"
#include "StateFactory.h"

#include <cassert>

void PipelineState::Init(ID3D12Device* device, D3D12_GRAPHICS_PIPELINE_STATE_DESC desc)
{
	auto result = device->CreateGraphicsPipelineState(
		&desc,
		IID_PPV_ARGS(_pipelineState.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		assert(0 && "パイプラインステートオブジェクトの作成に失敗!");
		return;
	}
	_pipelineState->SetName(L"pipeline_state");
	_rootSignature = desc.pRootSignature;
}

void PipelineState::CreateFromDesc(
	const Graphics::GfxDevice& device,
	const Graphics::MaterialDesc& desc,
	Graphics::RootSignatureRegistry& rootSignatureRegistry)
{
	auto rootSignature = rootSignatureRegistry.GetOrCreate(device, desc.rootSignatureDesc);

	Shader vs, ps;
	vs.LoadVS(desc.vertexShaderPath.c_str(), "vs");
	ps.LoadPS(desc.pixelShaderPath.c_str(), "ps");

	auto inputLayout = InputLayoutHelper::CreateInputLayout(desc.inputElements);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = rootSignature->Get();
	psoDesc.VS = vs.GetBytecode();
	psoDesc.PS = ps.GetBytecode();
	psoDesc.InputLayout = { inputLayout.data(), static_cast<UINT>(inputLayout.size()) };
	psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	psoDesc.RasterizerState = Graphics::StateFactory::GetRasterizerState(desc.rasterizerPreset);
	psoDesc.BlendState = Graphics::StateFactory::GetBlendState(desc.blendPreset);
	psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;    // カットなし
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;   // 三角形で描画
	psoDesc.NumRenderTargets = 1; // レンダーターゲットは一つ
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DepthStencilState = Graphics::StateFactory::GetDepthStencilState(desc.depthStencilPreset);

	auto result = device.Get()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(_pipelineState.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		assert(0 && "パイプラインステートオブジェクトの作成に失敗!");
		return;
	}

	_pipelineState->SetName(L"pipeline_state");
	_rootSignature = psoDesc.pRootSignature;
}