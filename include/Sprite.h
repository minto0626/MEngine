﻿#pragma once
#include <d3d12.h>
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Texture.h"
#include "Transform.h"
#include "Vector.h"
#include "ConstantBuffer.h"

class Sprite
{
private:
	VertexBuffer _vertexBuffer;
	IndexBuffer _indexBuffer;
	Texture _texture;
	Transform _transform;
	Vector2 _size;

	ConstantBuffer _constantBuffer;

	void InitVertexBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
	void InitIndexBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
	void InitTexture(ID3D12Device* device, ID3D12Resource* texture);
	void InitConstantBuffer(ID3D12Device* device);

public:
	void Init(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* texture, bool setNativeSize, float width = 0.0f, float height = 0.0f);
	void Update(Matrix cameraView);
	void Draw(ID3D12GraphicsCommandList* commandList);

	Transform* Transform() { return &_transform; }
};