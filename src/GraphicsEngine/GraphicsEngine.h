#pragma once
#include "Core/GfxDevice.h"
#include "Core/GfxSwapChain.h"
#include "Core/GfxCommandQueue.h"
#include "Core/GfxCommandContext.h"
#include "Core/GfxFence.h"
#include "Core/GraphicsContext.h"
#include "DescriptorHeap/DescriptorHeap.h"
#include "TextureLoader.h"
#include "Material/MaterialCache.h"
#include "Material/MaterialRegistry.h"
#include "Mesh/Mesh.h"
#include "Sprite/Sprite.h"
#include "Pipeline/RootSignature.h"
#include "Pipeline/PipelineState.h"
#include "Pipeline/RootSignatureRegistry.h"
#include "Resources/RenderTarget.h"
#include "Resources/DepthBuffer.h"
#include "Resources/Texture.h"
#include "Resources/ConstantBuffer.h"
#include "Renderer/Renderer.h"
#include "Renderer/MeshRenderer.h"
#include "Renderer/SpriteRenderer.h"
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
		std::unique_ptr<DescriptorHeap> dsv_heap;

		std::vector<std::unique_ptr<RenderTarget>> renderTargets;
		Color clearColor = Color::FromHex(0x6c9bd2);
		std::unique_ptr<DepthBuffer> depthBuffer;
		Camera camera2D;
		Camera camera3D;

		TextureLoader textureLoader;
		std::unique_ptr<MaterialCache> materialCache;
		std::unique_ptr<RootSignatureRegistry> rootSignatureRegistry;
		std::unique_ptr<MaterialRegistry> materialRegistry;
		std::vector<Renderer*> scene2DRenderers;
		std::vector<Renderer*> scene3DRenderers;

		const std::string worldMatParamName = "worldMat";
		const std::string mainTexParamName = "mainTex";
		std::shared_ptr<RootSignature> basicRootSignature;

		std::unique_ptr<Sprite> sprite;
		std::unique_ptr<Material> material;
		std::unique_ptr<Texture> texture;
		std::unique_ptr<ConstantBuffer> constantBuffer;
		Transform transform;
		std::unique_ptr<SpriteRenderer> spriteRenderer;

		std::unique_ptr<Sprite> sprite2;
		std::unique_ptr<Material> material2;
		std::unique_ptr<Texture> texture2;
		std::unique_ptr<ConstantBuffer> constantBuffer2;
		Transform transform2;
		std::unique_ptr<SpriteRenderer> spriteRenderer2;

		std::unique_ptr<Sprite> sprite3;
		std::unique_ptr<Material> material3;
		std::unique_ptr<Texture> texture3;
		std::unique_ptr<ConstantBuffer> constantBuffer3;
		Transform transform3;
		std::unique_ptr<SpriteRenderer> spriteRenderer3;

		std::unique_ptr<Mesh> mesh;
		std::unique_ptr<Texture> texture4;
		std::unique_ptr<ConstantBuffer> constantBuffer4;
		Transform transform4;
		std::unique_ptr<MeshRenderer> meshRenderer;

		IDXGIFactory6* CreateDXGIFactory();

	public:
		bool Initialize(HWND hwnd, SIZE& windowSize);
		void UnInitialize();
		void LoadContent();
		void Render();

	};
}