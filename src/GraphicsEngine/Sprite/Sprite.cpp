#include "Sprite.h"

namespace Graphics
{
	bool Sprite::Initialize(ID3D12Device* device, GfxCommandContext& commandContext, SpriteData& spriteData)
	{
		_vertexBuffer.Init(device, commandContext, spriteData.vertices.data(), spriteData.vertices.size(), sizeof(SpriteVertex));

		_indexBuffer.Init(device, commandContext, spriteData.indices.data(), spriteData.indices.size(), DXGI_FORMAT_R16_UINT);

		return true;
	}

	void Sprite::Draw(GfxCommandContext& commandContext)
	{
		commandContext.SetVertexBuffer(0, _vertexBuffer.GetView());
		commandContext.SetIndexBuffer(_indexBuffer.GetView());
		commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandContext.DrawIndexedInstanced(_indexBuffer.GetIndexNum());
	}
}