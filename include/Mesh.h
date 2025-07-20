#pragma once
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "GfxCommandContext.h"

namespace Graphics
{
	class Mesh
	{
	private:
		VertexBuffer _vertexBuffer;
		IndexBuffer _indexBuffer;

	public:
		bool Initialize(ID3D12Device* device, GfxCommandContext& commandContext);
		void Draw(GfxCommandContext& commandContext);

	};
}