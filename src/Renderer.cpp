#include "Renderer.h"

namespace Graphics
{
	void Renderer::Draw(GfxCommandContext* commandContext, Mesh* mesh, Material* material)
	{
		auto* commandList = commandContext->GetCommandList();

		auto* pso = material->GetPipelineState();
		commandContext->SetPipelineState(pso->Get());
		commandContext->SetRootSignature(pso->GetRootSignature());

		material->Bind(*commandContext);
		mesh->Draw(*commandContext);
	}
}