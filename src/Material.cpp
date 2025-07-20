#include "Material.h"

namespace Graphics
{
	void Material::UploadConstantBuffer(const void* data, UINT size)
	{
		_constantBuffer->Update(data, size);
	}

	void Material::Bind(GfxCommandContext& commandContext)
	{
		commandContext.SetGraphicsRootDescriptorTable(0, _constantBuffer->GetGPUHandle());
		commandContext.SetGraphicsRootDescriptorTable(1, _texture->GetGPUHandle());
	}
}