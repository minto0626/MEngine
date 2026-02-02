#include "GraphicsEngine.h"
#include "Shader/Shader.h"
#include "Pipeline/InputLayoutHelper.h"
#include "Pipeline/StateFactory.h"

#include "MEngine.h"
#include "GUI/GUISystem.h"

#include <wrl.h>
#include <Core/d3dx12.h>

using namespace Microsoft::WRL;

namespace Graphics
{
	ComPtr<IDXGIFactory6> GraphicsEngine::CreateDXGIFactory()
	{
		UINT dxgiFactoryFlag = 0;

#if _DEBUG
		ComPtr<ID3D12Debug> debugLayer = nullptr;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugLayer.ReleaseAndGetAddressOf()))))
		{
			dxgiFactoryFlag |= DXGI_CREATE_FACTORY_DEBUG;

			debugLayer->EnableDebugLayer();
		}
#endif
        HRESULT result;
		ComPtr<IDXGIFactory6> factory;
#if _DEBUG
		result = CreateDXGIFactory2(dxgiFactoryFlag, IID_PPV_ARGS(factory.ReleaseAndGetAddressOf()));
#else
		result = CreateDXGIFactory1(IID_PPV_ARGS(factory.ReleaseAndGetAddressOf()));
#endif
        if (FAILED(result))
        {
            return nullptr;
        }

		return factory;
	}

	bool GraphicsEngine::Initialize(HWND hwnd, SIZE& windowSize)
	{
		auto factory = CreateDXGIFactory();
		if (!device.Initialize(factory.Get())) { return false; }
		if (!commandQueue.Initialize(&device, D3D12_COMMAND_LIST_TYPE_DIRECT)) { return false; }
		if (!swapChain.Initialize(hwnd, windowSize, &commandQueue, factory.Get())) { return false; }
		if (!commandContext.Initialize(&device, &commandQueue)) { return false; }
		if (!fence.Initialize(&device)) { return false; }

		// モデルインポーター初期化
		modelImporter.Init();

		graphicsContext.device = &device;
		graphicsContext.commandQueue = &commandQueue;
		graphicsContext.commandContext = &commandContext;
		graphicsContext.fence = &fence;

		rtv_heap = std::make_unique<DescriptorHeap>(
			device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 32, false
		);
		dsv_heap = std::make_unique<DescriptorHeap>(
			device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 32, false
		);
		cbv_srv_uav_heap = std::make_unique<DescriptorHeap>(
			device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 64
		);

		DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
		swapChain.Get()->GetDesc1(&swapchainDesc);
		renderTargets.resize(swapchainDesc.BufferCount);
		for (UINT i = 0; i < renderTargets.size(); ++i)
		{
			renderTargets[i] = std::make_unique<RenderTarget>();
			renderTargets[i]->InitFromSwapChain(&device, &swapChain, *rtv_heap, buckBuffer_clearColor, i);
		}

		textureLoader.Init(&graphicsContext, cbv_srv_uav_heap.get());
		scene2DRenderers.clear();
		scene3DRenderers.clear();

		rootSignatureRegistry = std::make_unique<RootSignatureRegistry>();
		materialCache = std::make_unique<MaterialCache>(rootSignatureRegistry.get());
		materialRegistry = std::make_unique<MaterialRegistry>(&device, materialCache.get());

        // シャドウマップ用レンダリングターゲット作成
        shadowMapRenderTarget = std::make_unique<RenderTarget>();
        // 解像度は2048x2048
        shadowMapRenderTarget->InitDepth(
            device,
            2048,
            2048,
            DXGI_FORMAT_R32_TYPELESS,
            DXGI_FORMAT_D32_FLOAT,
            1.0f,
            *dsv_heap,
            DXGI_FORMAT_R32_FLOAT,
            cbv_srv_uav_heap.get()
        );

        // Gバッファ用レンダリングターゲット作成
        gBuffer[0] = std::make_unique<RenderTarget>();
        gBuffer[0]->InitColor(
            device,
            swapchainDesc.Width,
            swapchainDesc.Height,
            DXGI_FORMAT_R8G8B8A8_UNORM,   // Albedo
            gBuffer_clearColor,
            *rtv_heap,
            cbv_srv_uav_heap.get()
        );
        // 深度バッファはこれを使う
        gBuffer[0]->InitDepth(
            device,
            swapchainDesc.Width,
            swapchainDesc.Height,
            DXGI_FORMAT_R32_TYPELESS,
            DXGI_FORMAT_D32_FLOAT,
            1.0f,
            *dsv_heap
        );
        gBuffer[1] = std::make_unique<RenderTarget>();
        gBuffer[1]->InitColor(
            device,
            swapchainDesc.Width,
            swapchainDesc.Height,
            DXGI_FORMAT_R8G8B8A8_UNORM,   // Normal
            gBuffer_clearColor,
            *rtv_heap,
            cbv_srv_uav_heap.get()
        );
        gBuffer[2] = std::make_unique<RenderTarget>();
        gBuffer[2]->InitColor(
            device,
            swapchainDesc.Width,
            swapchainDesc.Height,
            DXGI_FORMAT_R32G32B32A32_FLOAT,   // Position
            gBuffer_clearColor,
            *rtv_heap,
            cbv_srv_uav_heap.get()
        );

        // オフスクリーンレンダリングターゲット作成
        offscreenRenderTarget = std::make_unique<RenderTarget>();
        offscreenRenderTarget->InitColor(
            device,
            swapchainDesc.Width,
            swapchainDesc.Height,
            swapchainDesc.Format,
            offsecreen_clearColor,
            *rtv_heap,
            cbv_srv_uav_heap.get()
        );

        // ポストプロセス用レンダリングターゲット作成
        postProcessRenderTarget = std::make_unique<RenderTarget>();
        postProcessRenderTarget->InitColor(
            device,
            swapchainDesc.Width,
            swapchainDesc.Height,
            swapchainDesc.Format,
            postProcess_clearColor,
            *rtv_heap,
            cbv_srv_uav_heap.get()
        );

        // ImGui 初期化
        if (!MEngine::GUI()->Initialize(hwnd, &device, cbv_srv_uav_heap.get(), swapchainDesc.Format))
        {
            return false;
        }

		return true;
	}

	void GraphicsEngine::UnInitialize()
	{
		commandQueue.Signal(fence);
		fence.Wait();
	}

	void GraphicsEngine::LoadContent()
	{
        // ポストプロセス
        {
            auto* mat = GetMaterial("Assets/RenderPass/PostProcess.mat");
            postProcessMat = mat;
            mat->SetTexture(GetRootParameterIndex("srcTex", *mat), offscreenRenderTarget->GetColorTexture());
        }
        // シャドウマップ
        {
            auto* mat = GetMaterial("Assets/RenderPass/ShadowMap.mat");
            shadowMapMat = mat;
            mat->SetConstantBuffer(GetRootParameterIndex(sceneDataParamName, *mat), sceneCB);
        }
        // ライト描画
        {
            auto* mat = GetMaterial("Assets/RenderPass/SceneLighting.mat");
            lightingMat = mat;
            mat->SetConstantBuffer(GetRootParameterIndex(sceneDataParamName, *mat), sceneCB);
            mat->SetTexture(GetRootParameterIndex("albedoTex", *mat), gBuffer[0]->GetColorTexture());
            mat->SetTexture(GetRootParameterIndex("normalTex", *mat), gBuffer[1]->GetColorTexture());
            mat->SetTexture(GetRootParameterIndex("positionTex", *mat), gBuffer[2]->GetColorTexture());
            mat->SetTexture(GetRootParameterIndex(shadowMapParamName, *mat), shadowMapRenderTarget->GetDepthTexture());
        }

		// テクスチャ取得のメモ
		{
			std::vector<ImportMeshData> meshDataList;
			std::vector<ImportMaterialData> materialDataList;
			if (!modelImporter.Load(L"Assets/3D/samples/teapot/teapot.fbx", meshDataList, materialDataList))
			{
				assert(0 && "モデル読み込み失敗");
				return;
			}
			ImportMeshData& meshDataSrc = meshDataList[0];
			if (materialDataList.size() > 0 &&
				materialDataList[meshDataSrc.materialIndex].useDiffuseTexture)
			{
                auto texture = GetTexture(materialDataList[meshDataSrc.materialIndex].diffuseTexturePath.c_str());
			}
		}
	}

	Sprite* GraphicsEngine::GetSprite(const std::string& path)
	{
        if (sprites.find(path) != sprites.end())
        {
            return sprites[path].get();
        }

		SpriteData spriteData;
		auto width = 100.0f;
		auto height = 100.0f;
		auto halfW = width * 0.5f;
		auto halfH = height * 0.5f;
		spriteData.vertices = {
			{{ -halfW,  halfH, 0.0f }, { 0.0f, 1.0f }},   // 左下
			{{ -halfW, -halfH, 0.0f }, { 0.0f, 0.0f }},   // 左上
			{{  halfW,  halfH, 0.0f }, { 1.0f, 1.0f }},   // 右下
			{{  halfW, -halfH, 0.0f }, { 1.0f, 0.0f }},   // 右上
		};
		spriteData.indices = {
			0, 1, 2,
			2, 1, 3,
		};
		auto sprite = std::make_unique<Sprite>();
		sprite->Initialize(device.Get(), commandContext, spriteData);
        auto ptr = sprite.get();
        sprites[path] = std::move(sprite);
		return ptr;
	}

	Mesh* GraphicsEngine::GetMesh(const std::wstring& path)
	{
        if (meshes.find(path) != meshes.end())
        {
            return meshes[path].get();
        }

		std::vector<ImportMeshData> meshDataList;
		std::vector<ImportMaterialData> materialDataList;
		if (!modelImporter.Load(path.c_str(), meshDataList, materialDataList))
		{
			assert(0 && "モデル読み込み失敗");
			return nullptr;
		}
		// 今はメッシュが一つだけの想定
		ImportMeshData& meshDataSrc = meshDataList[0];
		MeshData meshData;
		meshData.vertices = meshDataSrc.vertices;
		meshData.indices = meshDataSrc.indices;
		auto mesh = std::make_unique<Mesh>();
		mesh->Initialize(device.Get(), commandContext, meshData);
        auto ptr = mesh.get();
        meshes[path] = std::move(mesh);
		return ptr;
	}

	Material* GraphicsEngine::GetMaterial(const std::string& name)
	{
        ShaderParam shaderParam;
        auto material = materialRegistry->Get(name, shaderParam);

        // 定数バッファおよびテクスチャは、マテリアルが初めて作成されたときに設定されます。
        for (auto& constant : shaderParam.constants)
        {
            UINT size = static_cast<UINT>(constant.value.size()) * sizeof(constant.value[0]);
            auto constantBuffer = CreateConstantBuffer(size);
            constantBuffer->Update(constant.value.data(), size);
            material->SetConstantBuffer(GetRootParameterIndex(constant.paramName, *material), constantBuffer);
        }
        for (auto& texture : shaderParam.textures)
        {
            material->SetTexture(GetRootParameterIndex(texture.paramName, *material), GetTexture(texture.path));
        }

        return material;
	}

	UINT GraphicsEngine::GetRootParameterIndex(const std::string& name, const Material& mat)
	{
		auto& rootSignatureDesc = mat.GetDesc().shaderDesc.rootSignatureDesc;
		auto rootSignature = rootSignatureRegistry->GetOrCreate(device, rootSignatureDesc);
		return rootSignature->GetRootIndex(name);
	}

    std::shared_ptr<ConstantBuffer> GraphicsEngine::CreateConstantBuffer(UINT size)
	{
		auto constantBuffer = std::make_shared<ConstantBuffer>();
		constantBuffer->Init(device.Get(), cbv_srv_uav_heap.get(), size);
		return constantBuffer;
	}

    Texture* GraphicsEngine::GetTexture(const std::string& path)
	{
		return textureLoader.GetTexture(path.c_str());
	}

    void GraphicsEngine::InitSceneConstantBuffers(UINT canvasDataSize, UINT sceneDataSize)
    {
        canvasCB = CreateConstantBuffer(canvasDataSize);
        sceneCB = CreateConstantBuffer(sceneDataSize);
    }

    void GraphicsEngine::UpdateSceneConstantBuffer(void* buffer, UINT size)
    {
        sceneCB->Update(buffer, size);
    }

    void GraphicsEngine::UpdateCanvasConstantBuffer(void* buffer, UINT size)
    {
        canvasCB->Update(buffer, size);
    }

	void GraphicsEngine::RegisterSpriteRenderer(SpriteRenderer* spriteRenderer)
	{
		scene2DRenderers.push_back(spriteRenderer);
	}

	void GraphicsEngine::RegisterMeshRenderer(MeshRenderer* meshRenderer)
	{
		scene3DRenderers.push_back(meshRenderer);
	}

	void GraphicsEngine::Render(Scene* scene)
	{
        // 共有デスクリプタヒープをセット
        const DescriptorHeap* heaps[] = { cbv_srv_uav_heap.get() };
        graphicsContext.SetDescriptorHeaps(_countof(heaps), heaps);

        // シャドウマップを描画
        RenderShadowMap();

        // G-Bufferにシーンを描画
        RenderScene();

        // ライティングを描画
        RenderLighting();

        // ポストプロセスを描画
        RenderPostProcess();

        // バックバッファへ描画
        RenderBackBuffer(scene);

        graphicsContext.ExecuteCommand();
        graphicsContext.WaitGPU();

        graphicsContext.ResetCommand();

		swapChain.Present();
	}

    void GraphicsEngine::RenderShadowMap()
    {
        auto* renderTarget = shadowMapRenderTarget.get();

        graphicsContext.SetViewportAndScissor(renderTarget);

        graphicsContext.SetRenderTarget(renderTarget);
        graphicsContext.ClearRenderTarget(renderTarget);

        shadowMapMat->Bind(commandContext);

        for (auto& renderer : scene3DRenderers)
        {
            renderer->Draw(&commandContext);
        }
    }

    void GraphicsEngine::RenderScene()
    {
        const RenderTarget* gBufferRTs[] =
        {
            gBuffer[0].get(),   // Albedo
            gBuffer[1].get(),   // Normal
            gBuffer[2].get(),   // Position
        };

        for (auto& renderTarget : gBufferRTs)
        {
            graphicsContext.SetViewportAndScissor(renderTarget);
        }

        for (auto& renderTarget : gBufferRTs)
        {
            graphicsContext.TransitionShaderResourceToRenderTarget(renderTarget);
        }

        graphicsContext.SetRenderTargets(_countof(gBufferRTs), gBufferRTs);
        graphicsContext.ClearRenderTargets(_countof(gBufferRTs), gBufferRTs);

        bool setSceneCBV = false;
        bool setCanvasCBV = false;

        // 同じマテリアルでソートする。パイプラインの切り替えを最小限に抑えるため。
        std::sort(scene2DRenderers.begin(), scene2DRenderers.end(),
            [](const Renderer* a, const Renderer* b)
            {
                Material* materialA = a->GetMaterial();
                Material* materialB = b->GetMaterial();
                if (!materialA || !materialB) { return materialA < materialB; }
                return materialA->GetInstanceID() < materialB->GetInstanceID();
            });
        std::sort(scene3DRenderers.begin(), scene3DRenderers.end(),
            [](const Renderer* a, const Renderer* b)
            {
                Material* materialA = a->GetMaterial();
                Material* materialB = b->GetMaterial();
                if (!materialA || !materialB) { return materialA < materialB; }
                return materialA->GetInstanceID() < materialB->GetInstanceID();
            });

        Material* lastMaterial = nullptr;

        for (auto& renderer : scene3DRenderers)
        {
            Material* currentMaterial = renderer->GetMaterial();
            if (currentMaterial && currentMaterial != lastMaterial)
            {
                currentMaterial->Bind(commandContext);
                lastMaterial = currentMaterial;

                // シーンの共通定数バッファは一度だけセットします
                if (!setSceneCBV)
                {
                    // 3Dシーン用の定数バッファをセット
                    auto sceneCBVIndex = GetRootParameterIndex(sceneDataParamName, *currentMaterial);
                    commandContext.SetGraphicsRootDescriptorTable(sceneCBVIndex, sceneCB->GetGPUHandle());
                    setSceneCBV = true;
                }
            }
            renderer->Draw(&commandContext);
        }
        scene3DRenderers.clear();

        for (auto& renderer : scene2DRenderers)
        {
            Material* currentMaterial = renderer->GetMaterial();
            if (currentMaterial && currentMaterial != lastMaterial)
            {
                currentMaterial->Bind(commandContext);
                lastMaterial = currentMaterial;

                if (!setCanvasCBV)
                {
                    // 2Dキャンバス用の定数バッファをセット
                    auto canvasCBVIndex = GetRootParameterIndex(canvasDataParamName, *currentMaterial);
                    commandContext.SetGraphicsRootDescriptorTable(canvasCBVIndex, canvasCB->GetGPUHandle());
                    setCanvasCBV = true;
                }
            }
            renderer->Draw(&commandContext);
        }
        scene2DRenderers.clear();

        for (auto& renderTarget : gBufferRTs)
        {
            graphicsContext.TransitionRenderTargetToShaderResource(renderTarget);
        }
    }

    void GraphicsEngine::RenderLighting()
    {
        auto* renderTarget = offscreenRenderTarget.get();

        graphicsContext.SetViewportAndScissor(renderTarget);

        graphicsContext.TransitionShaderResourceToRenderTarget(renderTarget);

        graphicsContext.SetRenderTarget(renderTarget);
        graphicsContext.ClearRenderTarget(renderTarget);

        lightingMat->Bind(commandContext);
        commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandContext.DrawInstanced(3);

        graphicsContext.TransitionRenderTargetToShaderResource(renderTarget);
    }

    void GraphicsEngine::RenderPostProcess()
    {
        auto* renderTarget = postProcessRenderTarget.get();

        graphicsContext.SetViewportAndScissor(renderTarget);

        graphicsContext.TransitionShaderResourceToRenderTarget(renderTarget);

        graphicsContext.SetRenderTarget(renderTarget);
        graphicsContext.ClearRenderTarget(renderTarget);

        postProcessMat->Bind(commandContext);
        commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandContext.DrawInstanced(3);

        graphicsContext.TransitionRenderTargetToShaderResource(renderTarget);
    }

    void GraphicsEngine::RenderBackBuffer(Scene* scene)
    {
        UINT backBufferIndex = swapChain.Get()->GetCurrentBackBufferIndex();
        auto* renderTarget = renderTargets[backBufferIndex].get();

        graphicsContext.SetViewportAndScissor(renderTarget);

        graphicsContext.TransitionPresentToRenderTarget(renderTarget);

        graphicsContext.SetRenderTarget(renderTarget);
        graphicsContext.ClearRenderTarget(renderTarget);

        // GUIテスト描画
        MEngine::GUI()->NewFrame();
        static GameObject* selectGameObject = nullptr;
        float screen_width = static_cast<float>(renderTarget->GetWidth());
        float screen_height = static_cast<float>(renderTarget->GetHeight());
        MEngine::GUI()->DrawHierarchyWindow(Vector2(0.0f, 0.0f), Vector2(screen_width * 0.25f, screen_height), *scene, selectGameObject);
        MEngine::GUI()->DrawSceneViewWindow(Vector2(screen_width - screen_width * 0.75f, 0.0f), Vector2((screen_width - 32.0f) * 0.5f, (screen_height - 32.0f) * 0.5f), postProcessRenderTarget->GetColorTexture()->GetSRV());
        MEngine::GUI()->DrawInspectorWindow(Vector2(screen_width - screen_width * 0.25f, 0.0f), Vector2(screen_width * 0.25f, screen_height), selectGameObject);
        MEngine::GUI()->Render(&commandContext, cbv_srv_uav_heap.get());

        graphicsContext.TransitionRenderTargetToPresent(renderTarget);
    }
}
