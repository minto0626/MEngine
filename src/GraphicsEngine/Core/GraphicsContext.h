#pragma once
#include "GfxDevice.h"
#include "GfxCommandQueue.h"
#include "GfxCommandContext.h"
#include "GfxFence.h"

namespace Graphics
{
	class GraphicsContext
	{
	public:
		GfxDevice* device = nullptr;
		GfxCommandContext* commandContext = nullptr;
		GfxCommandQueue* commandQueue = nullptr;
		GfxFence* fence = nullptr;

		void ExecuteCommand();
		void WaitGPU();
		void ResetCommand();
	};
}