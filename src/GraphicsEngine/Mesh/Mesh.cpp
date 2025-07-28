#include "Mesh.h"
#include "Math/Vector.h"

namespace
{
	struct MeshVertex
	{
		Vector3 pos;
		Vector2 uv;
	};
}

namespace Graphics
{
	bool Mesh::Initialize(ID3D12Device* device, GfxCommandContext& commandContext)
	{
		auto halfW = 300.0f * 0.5f;
		auto halfH = 300.0f * 0.5f;

		const MeshVertex vertexData[] = {
			{{ -halfW,  halfH, 0.0f }, { 0.0f, 1.0f }},   // 左下
			{{ -halfW, -halfH, 0.0f }, { 0.0f, 0.0f }},   // 左上
			{{  halfW,  halfH, 0.0f }, { 1.0f, 1.0f }},   // 右下
			{{  halfW, -halfH, 0.0f }, { 1.0f, 0.0f }},   // 右上
		};
		_vertexBuffer.Init(device, commandContext, &vertexData, _countof(vertexData), sizeof(MeshVertex));

		const unsigned short indices[] = {
			0, 1, 2,
			2, 1, 3,
		};
		_indexBuffer.Init(device, commandContext, &indices, _countof(indices), DXGI_FORMAT_R16_UINT);

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