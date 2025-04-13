#pragma once
#include <d3d12.h>

#include "Shader.h"
#include "InputLayoutHelper.h"
#include "PipelineState.h"
#include "RootSignature.h"

class Sprite;

class SpriteRenderer
{
private:
	Shader _vertexShader;
	Shader _pixelShader;
	RootSignature rootSignature;
	PipelineState pipelineState;

	void InitRootSignature(ID3D12Device* device);
	void InitPipelineState(ID3D12Device* device);

public:
	void Init(ID3D12Device* device);
	void Render(ID3D12GraphicsCommandList* commandList);

};