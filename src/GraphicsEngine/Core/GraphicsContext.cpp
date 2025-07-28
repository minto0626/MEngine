#include "GraphicsContext.h"

namespace Graphics
{
    void GraphicsContext::ExecuteCommand()
    {
        commandContext->Close();
        ID3D12CommandList* commandLists[] = { commandContext->GetCommandList() };
        commandQueue->Get()->ExecuteCommandLists(_countof(commandLists), commandLists);
    }

    void GraphicsContext::WaitGPU()
    {
        commandQueue->Signal(*fence);
        fence->Wait();
    }

    void GraphicsContext::ResetCommand()
    {
        commandContext->Reset();
    }
}