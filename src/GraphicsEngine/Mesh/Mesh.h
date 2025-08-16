#pragma once
#include "Resources/VertexBuffer.h"
#include "Resources/IndexBuffer.h"
#include "Core/GfxCommandContext.h"
#include "Math/Vector.h"

#include <vector>

namespace Graphics
{
	struct MeshVertex
	{
		Vector3 pos;
		Vector2 uv;
	};

	struct MeshData
	{
		std::vector<MeshVertex> vertices;
		std::vector<unsigned short> indices;
	};

	class Mesh
	{
	private:
		VertexBuffer _vertexBuffer;
		IndexBuffer _indexBuffer;

	public:
		bool Initialize(ID3D12Device* device, GfxCommandContext& commandContext, MeshData& meshData);
		void Draw(GfxCommandContext& commandContext);

	};
}