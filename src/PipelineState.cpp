#include "PipelineState.h"
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
	_pipelineState->SetName(L"PipelineState");
}