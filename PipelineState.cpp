#include "PipelineState.h"
#include <cassert>

void PipelineState::Init(ID3D12Device* device, D3D12_GRAPHICS_PIPELINE_STATE_DESC desc)
{
	auto result = device->CreateGraphicsPipelineState(
		&desc,
		IID_PPV_ARGS(_pipelineState.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		assert(0 && "�p�C�v���C���X�e�[�g�I�u�W�F�N�g�̍쐬�Ɏ��s!");
		return;
	}
	_pipelineState->SetName(L"PipelineState");
}