#include "Mesh.h"

namespace Graphics
{
	bool Mesh::Initialize(ID3D12Device* device, GfxCommandContext& commandContext, MeshData& meshData)
	{
		_vertexBuffer.Init(device, commandContext, meshData.vertices.data(), meshData.vertices.size(), sizeof(MeshVertex));

		// インデックスバッファーのフォーマットを頂点数で決定
		if (meshData.indices.size() < (std::numeric_limits<uint16_t>::max)())
		{
			auto dstIndices = reinterpret_cast<uint16_t*>(meshData.indices.data());
			for (auto i = 0; i < meshData.indices.size(); ++i)
			{
				dstIndices[i] = static_cast<uint16_t>(meshData.indices[i]);
			}
			_indexBuffer.Init(device, commandContext, dstIndices, meshData.indices.size(), DXGI_FORMAT_R16_UINT);
		}
		else
		{
			_indexBuffer.Init(device, commandContext, meshData.indices.data(), meshData.indices.size(), DXGI_FORMAT_R32_UINT);
		}

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