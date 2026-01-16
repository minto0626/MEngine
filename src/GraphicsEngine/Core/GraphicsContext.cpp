#include "GraphicsContext.h"
#include "d3dx12.h"
#include "Resources/RenderTarget.h"

namespace Graphics
{
    void GraphicsContext::SetViewportAndScissor(const RenderTarget* renderTarget)
    {
        auto viewport = CD3DX12_VIEWPORT
        (
            0.0f,
            0.0f,
            static_cast<FLOAT>(renderTarget->GetWidth()),
            static_cast<FLOAT>(renderTarget->GetHeight())
        );
        auto scissor = CD3DX12_RECT(0, 0, renderTarget->GetWidth(), renderTarget->GetHeight());
        commandContext->SetViewport(viewport);
        commandContext->SetScissorRect(scissor);
    }

    void GraphicsContext::SetRenderTarget(const RenderTarget* renderTarget)
    {
        SetRenderTargets(1, &renderTarget);
    }

    void GraphicsContext::SetRenderTargets(UINT numRenderTargets, const RenderTarget* renderTargets[])
    {
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles;
        for (UINT i = 0; i < numRenderTargets; ++i)
        {
            if (renderTargets[i]->GetRTV().IsValid())
            {
                rtvHandles.push_back(renderTargets[i]->GetRTV().cpuHandle);
            }
        }

        if (rtvHandles.size() == 0)
        {
            numRenderTargets = 0;
        }

        // 深度ステンシルは、最初のレンダーターゲットのものを使用する
        auto rtvHandle = numRenderTargets > 0 ? rtvHandles.data() : nullptr;
        auto dsvHandle = renderTargets[0]->GetDSV().cpuHandle;
        commandContext->SetRenderTargets(numRenderTargets, rtvHandle, renderTargets[0]->GetDSV().IsValid(), &dsvHandle);
    }

    void GraphicsContext::ClearRenderTarget(const RenderTarget* renderTarget)
    {
        const RenderTarget* rtv[] = { renderTarget };
        ClearRenderTargets(1, rtv);
    }

    void GraphicsContext::ClearRenderTargets(UINT numRenderTargets, const RenderTarget* renderTargets[])
    {
        for (UINT i = 0; i < numRenderTargets; ++i)
        {
            if (renderTargets[i]->GetRTV().IsValid())
            {
                Color& clearColor = *renderTargets[i]->GetRTVClearColor();
                float color[4] = { clearColor.r, clearColor.g, clearColor.b, clearColor.a };
                commandContext->ClearRenderTargetView(renderTargets[i]->GetRTV().cpuHandle, color);
            }
        }

        if (renderTargets[0]->GetDSV().IsValid())
        {
            commandContext->ClearDepthStencilView(renderTargets[0]->GetDSV().cpuHandle, D3D12_CLEAR_FLAG_DEPTH, renderTargets[0]->GetDSVClearValue(), 0);
        }
    }

    void GraphicsContext::SetDescriptorHeaps(UINT numHeaps, const DescriptorHeap* heaps[])
    {
        std::vector<ID3D12DescriptorHeap*> dpHeaps;
        for (UINT i = 0; i < numHeaps; ++i)
        {
            dpHeaps.push_back(heaps[i]->GetHeap());
        }
        commandContext->SetDescriptorHeaps(numHeaps, dpHeaps.data());
    }

    void GraphicsContext::TransitionShaderResourceToRenderTarget(const RenderTarget* renderTarget)
    {
        commandContext->ResourceBarrier(
            renderTarget->GetColorBuffer(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET);
    }

    void GraphicsContext::TransitionRenderTargetToShaderResource(const RenderTarget* renderTarget)
    {
        commandContext->ResourceBarrier(
            renderTarget->GetColorBuffer(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }

    void GraphicsContext::TransitionRenderTargetToPresent(const RenderTarget* renderTarget)
    {
        commandContext->ResourceBarrier(
            renderTarget->GetColorBuffer(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT);
    }

    void GraphicsContext::TransitionPresentToRenderTarget(const RenderTarget* renderTarget)
    {
        commandContext->ResourceBarrier(
            renderTarget->GetColorBuffer(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET);
    }

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
