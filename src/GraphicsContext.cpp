#include "GraphicsContext.h"

void GraphicsContext::ExecuteCommand()
{
    commandList->Close();

    ID3D12CommandList* commandLists[] = { commandList };
    commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
}

void GraphicsContext::WaitDraw()
{
    commandQueue->Signal(fence, ++*fenceValue);

    if (fence->GetCompletedValue() != *fenceValue)
    {
        fence->SetEventOnCompletion(*fenceValue, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
    }
}

void GraphicsContext::ResetCommand()
{
    commandAllocator->Reset();
    commandList->Reset(commandAllocator, nullptr);
}