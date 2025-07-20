#pragma once
#include "GfxDevice.h"
#include "GfxSwapChain.h"
#include "GfxCommandQueue.h"
#include "GfxCommandContext.h"
#include "GfxFence.h"
#include "GraphicsContext.h"
#include "DescriptorHeap.h"
#include "TextureLoader.h"
#include "Mesh.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "Texture.h"
#include "ConstantBuffer.h"
#include "Renderer.h"
#include "Camera.h"
#include "Transform.h"
#include <memory>
#include <vector>
#include <windows.h>
#include <wrl.h>

namespace Graphics
{
	class GraphicsEngine
	{
	private:
		GfxDevice device;
		GfxCommandQueue commandQueue;
		GfxSwapChain swapChain;
		GfxCommandContext commandContext;
		GfxFence fence;
		GraphicsContext graphicsContext;
		std::unique_ptr<DescriptorHeap> cbv_srv_uav_heap;
		std::unique_ptr<DescriptorHeap> rtv_heap;

		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> renderTargets;
		std::vector<DescriptorHandle> rtvHandles;
		float clearColor[4] = { 0.0f, 0.0f, 0.4f, 1.0f };
		Camera camera2D;

		TextureLoader textureLoader;

		std::unique_ptr<Mesh> mesh;
		std::unique_ptr<Material> material;
		std::unique_ptr<RootSignature> rootSignature;
		std::unique_ptr<PipelineState> pipelineState;
		std::unique_ptr<Texture> texture;
		std::unique_ptr<ConstantBuffer> constantBuffer;
		Transform transform;
		std::unique_ptr<Renderer> renderer;

		IDXGIFactory6* CreateDXGIFactory();

	public:
		bool Initialize(HWND hwnd, SIZE& windowSize);
		void UnInitialize();
		void LoadContent();
		void Render();

	};
}