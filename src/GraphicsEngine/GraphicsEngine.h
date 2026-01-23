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
#include "ModelImporter.h"
#include "Mesh/Mesh.h"
#include "Sprite/Sprite.h"
#include "Pipeline/RootSignature.h"
#include "Pipeline/PipelineState.h"
#include "Pipeline/RootSignatureRegistry.h"
#include "Resources/RenderTarget.h"
#include "Resources/Texture.h"
#include "Resources/ConstantBuffer.h"
#include "Renderer/Renderer.h"
#include "Renderer/MeshRenderer.h"
#include "Renderer/SpriteRenderer.h"
#include "Scene/Transform.h"
#include "Scene/GameObject.h"
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

        std::unique_ptr<RenderTarget> shadowMapRenderTarget;
        std::unique_ptr<RenderTarget> gBuffer[3];   // 0:Albedo, 1:Normal, 2:Position
        Color gBuffer_clearColor = Color::Black();
        std::unique_ptr<RenderTarget> offscreenRenderTarget;
        Color offsecreen_clearColor = Color::Black();
        std::unique_ptr<RenderTarget> postProcessRenderTarget;
        Color postProcess_clearColor = Color::Black();
		std::vector<std::unique_ptr<RenderTarget>> renderTargets;
        Color buckBuffer_clearColor = Color::FromHex(0x6c9bd2);

		ModelImporter modelImporter;
		TextureLoader textureLoader;
		std::unique_ptr<MaterialCache> materialCache;
		std::unique_ptr<RootSignatureRegistry> rootSignatureRegistry;
		std::unique_ptr<MaterialRegistry> materialRegistry;
        std::unordered_map<std::string, std::unique_ptr<Sprite>> sprites;

		std::vector<Renderer*> scene2DRenderers;
		std::vector<Renderer*> scene3DRenderers;

        const std::string canvasDataParamName = "canvasCB";
        const std::string sceneDataParamName = "sceneCB";
		const std::string worldMatParamName = "worldMat";
        const std::string shadowMapParamName = "shadowMap";

        std::shared_ptr<ConstantBuffer> canvasCB = nullptr;
        std::shared_ptr<ConstantBuffer> sceneCB = nullptr;

        Microsoft::WRL::ComPtr<IDXGIFactory6> CreateDXGIFactory();

	public:
		bool Initialize(HWND hwnd, SIZE& windowSize);
		void UnInitialize();
		void LoadContent();
		Sprite* GetSprite(const std::string& path);
		Mesh* GetMesh(const std::wstring& path);
		Material* GetMaterial(const std::string& name);
		UINT GetRootParameterIndex(const std::string& name, const Material& mat);
        std::shared_ptr<ConstantBuffer> CreateConstantBuffer(size_t size);
		Texture* GetTexture(const std::string& path);
        void InitSceneConstantBuffers(size_t canvasDataSize, size_t sceneDataSize);
        void UpdateSceneConstantBuffer(void* buffer, UINT size);
        void UpdateCanvasConstantBuffer(void* buffer, UINT size);
		void RegisterSpriteRenderer(SpriteRenderer* spriteRenderer);
		void RegisterMeshRenderer(MeshRenderer* meshRenderer);
		void Render(class Scene* scene);
        void RenderShadowMap();
        void RenderScene();
        void RenderLighting();
        void RenderPostProcess();
        void RenderBackBuffer(class Scene* scene);

	};
}
