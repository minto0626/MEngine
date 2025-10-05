#pragma once
#include "Core/GfxCommandContext.h"
#include "Resources/VertexBuffer.h"
#include "Resources/IndexBuffer.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"

#include <vector>

namespace Graphics
{
	struct SpriteVertex
	{
		Vector3 pos;
		Vector2 uv;
	};

	struct SpriteData
	{
		std::vector<SpriteVertex> vertices;
		std::vector<unsigned short> indices;
	};

	struct ScreenConstantBuffer
	{
		Matrix worldMatrix;
	};

	class Sprite
	{
	private:
		VertexBuffer _vertexBuffer;
		IndexBuffer _indexBuffer;

	public:
		bool Initialize(ID3D12Device* device, GfxCommandContext& commandContext, SpriteData& spriteData);
		void Draw(GfxCommandContext& commandContext);

	};
}