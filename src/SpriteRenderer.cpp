#include "SpriteRenderer.h"
#include "d3dx12.h"
#include "Sprite.h"

void SpriteRenderer::InitRootSignature(ID3D12Device* device)
{
    rootSignature.AddDescriptorTable(1, 0, D3D12_SHADER_VISIBILITY_VERTEX, D3D12_DESCRIPTOR_RANGE_TYPE_CBV);
    rootSignature.AddDescriptorTable(1, 0, D3D12_SHADER_VISIBILITY_PIXEL, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
    rootSignature.AddStaticSampler(0, D3D12_SHADER_VISIBILITY_PIXEL);
    rootSignature.Build(device);
}

void SpriteRenderer::InitPipelineState(ID3D12Device* device)
{
    _vertexShader.LoadVS(L"Assets/shader/BasicVertexShader.hlsl", "vs");
    _pixelShader.LoadPS(L"Assets/shader/BasicPixelShader.hlsl", "ps");

    auto inputLayout = InputLayoutHelper::CreateInputLayout(
        {
            { "POSITION", DXGI_FORMAT_R32G32B32_FLOAT },
            { "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT },
        }
    );

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc = {};
    pipelineStateDesc.pRootSignature = rootSignature.Get();
    pipelineStateDesc.VS = _vertexShader.GetBytecode();
    pipelineStateDesc.PS = _pixelShader.GetBytecode();
    pipelineStateDesc.InputLayout = { inputLayout.data(), static_cast<UINT>(inputLayout.size()) };
    pipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    pipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    pipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;  // カリングしない
    pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    pipelineStateDesc.BlendState.AlphaToCoverageEnable = true;  // アルファテストする
    pipelineStateDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;    // カットなし
    pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;   // 三角形で描画
    pipelineStateDesc.NumRenderTargets = 1; // レンダーターゲットは一つ
    pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pipelineStateDesc.SampleDesc.Count = 1;
    pipelineStateDesc.SampleDesc.Quality = 0;
    pipelineState.Init(device, pipelineStateDesc);
}

void SpriteRenderer::Init(ID3D12Device* device)
{
	InitRootSignature(device);
	InitPipelineState(device);
}

void SpriteRenderer::Render(ID3D12GraphicsCommandList* commandList)
{
    commandList->SetPipelineState(pipelineState.Get());
    commandList->SetGraphicsRootSignature(rootSignature.Get());
}