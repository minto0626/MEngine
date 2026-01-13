#pragma once
#include "GfxDevice.h"
#include "GfxCommandQueue.h"
#include "GfxCommandContext.h"
#include "GfxFence.h"

#include "Resources/RenderTarget.h"
#include "DescriptorHeap/DescriptorHeap.h"
#include "Math/Color.h"

namespace Graphics
{
	class GraphicsContext
	{
	public:
		GfxDevice* device = nullptr;
		GfxCommandContext* commandContext = nullptr;
		GfxCommandQueue* commandQueue = nullptr;
		GfxFence* fence = nullptr;

        void SetViewportAndScissor(const RenderTarget* renderTarget);
        void SetRenderTarget(const RenderTarget* renderTarget);
        void SetRenderTargets(UINT numRTs, const RenderTarget* renderTargets[]);
        void ClearRenderTarget(const RenderTarget* renderTarget, const Color& clearColor);
        void ClearRenderTargets(UINT numRenderTargets, const RenderTarget* renderTargets[], const Color& clearColor);
        void SetDescriptorHeaps(UINT numHeaps, const DescriptorHeap* heaps[]);
        void TransitionShaderResourceToRenderTarget(const RenderTarget* renderTarget);
        void TransitionRenderTargetToShaderResource(const RenderTarget* renderTarget);
        void TransitionRenderTargetToPresent(const RenderTarget* renderTarget);
        void TransitionPresentToRenderTarget(const RenderTarget* renderTarget);

		void ExecuteCommand();
		void WaitGPU();
		void ResetCommand();
	};
}
