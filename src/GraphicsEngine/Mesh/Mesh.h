#pragma once
#include "Resources/VertexBuffer.h"
#include "Resources/IndexBuffer.h"
#include "Core/GfxCommandContext.h"

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