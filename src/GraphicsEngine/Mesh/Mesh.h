#pragma once
#include "Resources/VertexBuffer.h"
#include "Resources/IndexBuffer.h"
#include "Core/GfxCommandContext.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"

#include <vector>

namespace Graphics
{
	struct MeshVertex
	{
		Vector3 pos;
		Vector3 normal;
		Vector2 uv;
	};

	struct MeshData
	{
		std::vector<MeshVertex> vertices;
		std::vector<unsigned short> indices;
	};

	struct SceneConstantBuffer
	{
		Matrix worldMatrix;
		Matrix viewMatrix;
		Matrix projectionMatrix;
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