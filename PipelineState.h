#pragma once
#include <d3d12.h>
#include <wrl.h>

class PipelineState
{
private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipelineState;

public:
	void Init(
		ID3D12Device* device,
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc);
	ID3D12PipelineState* Get() const { return _pipelineState.Get(); }
};