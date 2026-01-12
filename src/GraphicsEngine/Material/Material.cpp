#include "Material.h"

namespace Graphics
{
	Material::Material(UINT64 instanceID) :
		_instanceID(instanceID)
	{
	}

	void Material::SetConstantBuffer(UINT rootIndex, ConstantBuffer* constantBuffer)
	{
		_constantBuffers[rootIndex] = constantBuffer;
	}

	void Material::SetTexture(UINT rootIndex, Texture* texture)
	{
		_textures[rootIndex] = texture;
	}

	void Material::UploadConstantBuffer(UINT rootIndex, const void* data, UINT size)
	{
		auto it = _constantBuffers.find(rootIndex);
		if (it != _constantBuffers.end())
		{
			it->second->Update(data, size);
		}
	}

	void Material::Bind(GfxCommandContext& commandContext)
	{
		commandContext.SetPipelineState(_pipelineState->Get());
		commandContext.SetRootSignature(_pipelineState->GetRootSignature());

		for (auto& [rootIndex, buffer] : _constantBuffers)
		{
			commandContext.SetGraphicsRootDescriptorTable(rootIndex, buffer->GetGPUHandle());
		}

		for (auto& [rootIndex, texture] : _textures)
		{
			commandContext.SetGraphicsRootDescriptorTable(rootIndex, texture->GetSRV().gpuHandle);
		}
	}
}
