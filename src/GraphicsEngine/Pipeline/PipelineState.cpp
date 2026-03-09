#include "PipelineState.h"
#include "GraphicsEngine/GraphicsEngine.h"
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
    auto& shaderDesc = desc.shaderDesc;

	auto rootSignature = rootSignatureRegistry.GetOrCreate(device, shaderDesc.rootSignatureDesc);

    Shader vs, ps;

	auto inputLayout = InputLayoutHelper::CreateInputLayout(shaderDesc.inputElements);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = rootSignature->Get();
    if (!shaderDesc.vertexShaderPath.empty())
    {
        vs.LoadVS(shaderDesc.vertexShaderPath.c_str(), "vs");
        psoDesc.VS = vs.GetBytecode();
    }
    if (!shaderDesc.pixelShaderPath.empty())
    {
	    ps.LoadPS(shaderDesc.pixelShaderPath.c_str(), "ps");
	    psoDesc.PS = ps.GetBytecode();
    }
	psoDesc.InputLayout = { inputLayout.data(), static_cast<UINT>(inputLayout.size()) };
	psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	psoDesc.RasterizerState = Graphics::StateFactory::GetRasterizerState(desc.rasterizerPreset);
	psoDesc.BlendState = Graphics::StateFactory::GetBlendState(desc.blendPreset);
	psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;    // カットなし
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;   // 三角形で描画
	psoDesc.NumRenderTargets = static_cast<UINT>(shaderDesc.rtvFormats.size());
    for (size_t i = 0; i < shaderDesc.rtvFormats.size() && i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
    {
        psoDesc.RTVFormats[i] = shaderDesc.rtvFormats[i];
    }
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.DepthStencilState = Graphics::StateFactory::GetDepthStencilState(desc.depthStencilPreset);
	psoDesc.DSVFormat = desc.depthStencilPreset == Graphics::DepthStencilPreset::DepthEnable ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_UNKNOWN;

	auto result = device.Get()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(_pipelineState.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		assert(0 && "パイプラインステートオブジェクトの作成に失敗!");
		return;
	}

	_pipelineState->SetName(L"pipeline_state");
	_rootSignature = psoDesc.pRootSignature;
}
