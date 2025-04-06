#include "Sprite.h"

namespace
{
    struct MeshVertex
    {
        Vector3 pos;
        Vector2 uv;
    };
}

void Sprite::InitVertexBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
    const MeshVertex vertexData[] = {
        {{ -0.4f, -0.7f, 1.0f }, { 0.0f, 1.0f }},   // ¶‰º
        {{ -0.4f,  0.7f, 1.0f }, { 0.0f, 0.0f }},   // ¶ã
        {{  0.4f, -0.7f, 1.0f }, { 1.0f, 1.0f }},   // ‰E‰º
        {{  0.4f,  0.7f, 1.0f }, { 1.0f, 0.0f }},   // ‰Eã
    };
    _vertexBuffer.Init(device, commandList, &vertexData, _countof(vertexData), sizeof(MeshVertex));
}

void Sprite::InitIndexBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
    const unsigned short indices[] = {
        0, 1, 2,
        2, 1, 3,
    };
    _indexBuffer.Init(device, commandList, &indices, _countof(indices), DXGI_FORMAT_R16_UINT);
}

void Sprite::InitTexture(ID3D12Device* device, ID3D12Resource* texture)
{
    const char* texfilePath = "Assets/texture/free_ei.png";
    _texture.Init(device, texture);
}

void Sprite::InitConstantBuffer(ID3D12Device* device)
{
    _constantBuffer.Init(device, sizeof(_transform.GetWorldMatrix()));
    auto world = _transform.GetWorldMatrix();
    _constantBuffer.Update(&world, sizeof(world));
}

void Sprite::Init(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* texture)
{
    InitVertexBuffer(device, commandList);
    InitIndexBuffer(device, commandList);
    InitTexture(device, texture);
    InitConstantBuffer(device);
}

void Sprite::Update()
{
    auto pos = _transform.GetPos();
    pos.SetX(pos.GetX() + (1.0f / 30.0f) * 0.1f);
    _transform.SetPos(pos);
    auto world = _transform.GetWorldMatrix();
    _constantBuffer.Update(&world, sizeof(world));
}

void Sprite::Draw(ID3D12GraphicsCommandList* commandList)
{
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList->IASetVertexBuffers(0, 1, &_vertexBuffer.GetView());
    commandList->IASetIndexBuffer(&_indexBuffer.GetView());

    ID3D12DescriptorHeap* constantHeaps[] = { _constantBuffer.GetHeap() };
    commandList->SetDescriptorHeaps(1, constantHeaps);
    commandList->SetGraphicsRootDescriptorTable(0, _constantBuffer.GetGPUHandle());

    ID3D12DescriptorHeap* textureHeaps[] = { _texture.GetHeap() };
    commandList->SetDescriptorHeaps(1, textureHeaps);
    commandList->SetGraphicsRootDescriptorTable(1, _texture.GetGPUHandle());

    commandList->DrawIndexedInstanced(_indexBuffer.GetIndexNum(), 1, 0, 0, 0);
}