#include "Mesh.h"

namespace Graphics
{
	bool Mesh::Initialize(ID3D12Device* device, GfxCommandContext& commandContext, MeshData& meshData)
	{
		_vertexBuffer.Init(device, commandContext, meshData.vertices.data(), meshData.vertices.size(), sizeof(MeshVertex));

		_indexBuffer.Init(device, commandContext, meshData.indices.data(), meshData.indices.size(), DXGI_FORMAT_R16_UINT);

		return true;
	}

	void Mesh::Draw(GfxCommandContext& commandContext)
	{
		commandContext.SetVertexBuffer(0, _vertexBuffer.GetView());
		commandContext.SetIndexBuffer(_indexBuffer.GetView());
		commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandContext.DrawIndexedInstanced(_indexBuffer.GetIndexNum());
	}
}