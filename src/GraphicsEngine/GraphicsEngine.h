#pragma once
#include "Core/GfxDevice.h"
#include "Core/GfxSwapChain.h"
#include "Core/GfxCommandQueue.h"
#include "Core/GfxCommandContext.h"
#include "Core/GfxFence.h"
#include "Core/GraphicsContext.h"
#include "DescriptorHeap/DescriptorHeap.h"
#include "TextureLoader.h"
#include "Mesh/Mesh.h"
#include "Pipeline/RootSignature.h"
#include "Pipeline/PipelineState.h"
#include "Resources/Texture.h"
#include "Resources/ConstantBuffer.h"
#include "Renderer/Renderer.h"
#include "Renderer/MeshRenderer.h"
#include "Scene/Camera.h"
#include "Scene/Transform.h"
#include "Math/Color.h"

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
		Color clearColor = Color::FromHex(0x6c9bd2);
		Camera camera2D;

		TextureLoader textureLoader;
		std::vector<Renderer*> sceneRenderers;

		std::unique_ptr<Mesh> mesh;
		std::unique_ptr<Material> material;
		std::unique_ptr<RootSignature> rootSignature;
		std::unique_ptr<PipelineState> pipelineState;
		std::unique_ptr<Texture> texture;
		std::unique_ptr<ConstantBuffer> constantBuffer;
		Transform transform;
		std::unique_ptr<MeshRenderer> meshRenderer;
		const std::string worldMatParamName = "worldMat";
		const std::string mainTexParamName = "mainTex";

		std::unique_ptr<Mesh> mesh2;
		std::unique_ptr<Material> material2;
		std::unique_ptr<RootSignature> rootSignature2;
		std::unique_ptr<PipelineState> pipelineState2;
		std::unique_ptr<Texture> texture2;
		std::unique_ptr<ConstantBuffer> constantBuffer2;
		Transform transform2;
		std::unique_ptr<MeshRenderer> meshRenderer2;

		std::unique_ptr<Mesh> mesh3;
		std::unique_ptr<Material> material3;
		std::unique_ptr<RootSignature> rootSignature3;
		std::unique_ptr<PipelineState> pipelineState3;
		std::unique_ptr<Texture> texture3;
		std::unique_ptr<ConstantBuffer> constantBuffer3;
		Transform transform3;
		std::unique_ptr<MeshRenderer> meshRenderer3;

		IDXGIFactory6* CreateDXGIFactory();

	public:
		bool Initialize(HWND hwnd, SIZE& windowSize);
		void UnInitialize();
		void LoadContent();
		void Render();

	};
}