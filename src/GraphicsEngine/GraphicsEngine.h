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
#include "Scene/Light.h"

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

        std::unique_ptr<RenderTarget> offscreenRenderTarget;
        std::unique_ptr<RenderTarget> shadowMapRenderTarget;
		std::vector<std::unique_ptr<RenderTarget>> renderTargets;
		Color clearColor = Color::FromHex(0x6c9bd2);

		ModelImporter modelImporter;
		TextureLoader textureLoader;
		std::unique_ptr<MaterialCache> materialCache;
		std::unique_ptr<RootSignatureRegistry> rootSignatureRegistry;
		std::unique_ptr<MaterialRegistry> materialRegistry;
		std::vector<Renderer*> scene2DRenderers;
		std::vector<Renderer*> scene3DRenderers;

        const std::string sceneDataParamName = "sceneCB";
		const std::string worldMatParamName = "worldMat";
        const std::string shadowMapParamName = "shadowMap";

        ConstantBuffer* sceneCB = nullptr;

        struct SceneCameraData
        {
            Matrix viewMatrix;
            Matrix projectionMatrix;
            Vector3 cameraPosition;
            float pad0;
        };

        struct SceneLightData
        {
            Matrix lightViewMatrix;
            Vector3 lightDirection;
            float pad0;
        };

        struct SceneConstantBuffer
        {
            SceneCameraData camera;
            SceneLightData light;
        };

		IDXGIFactory6* CreateDXGIFactory();

	public:
		bool Initialize(HWND hwnd, SIZE& windowSize);
		void UnInitialize();
		void LoadContent();
		Sprite* GetSprite(const std::string& path);
		Mesh* GetMesh(const std::wstring& path);
		Material* GetMaterial(const std::string& name);
		UINT GetRootParameterIndex(const std::string& name, const Material& mat);
		ConstantBuffer* CreateConstantBuffer(size_t size);
		Texture* GetTexture(const std::string& path);
		void RegisterSpriteRenderer(SpriteRenderer* spriteRenderer);
		void RegisterMeshRenderer(MeshRenderer* meshRenderer);
		void Render(class Camera* camera2D, class Camera* camera3D, Light* light);
        void RenderShadowMap(class Camera* camera3D, Light* light);
        void RenderScene(class Camera* camera2D, class Camera* camera3D, Light* light);
        void RenderBackBuffer();

	};
}
