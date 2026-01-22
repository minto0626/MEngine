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
        RootSignatureDesc basic2DRootDesc;
        {
            basic2DRootDesc.params.push_back({ canvasDataParamName, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX });
            basic2DRootDesc.params.push_back({ worldMatParamName, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_VERTEX });
            basic2DRootDesc.params.push_back({ "mainTex", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL });
            basic2DRootDesc.staticSamplers.push_back({ 0, D3D12_SHADER_VISIBILITY_PIXEL, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_FILTER_ANISOTROPIC });
        }
        std::wstring basic2DVSPath = L"Assets/shader/BasicVertexShader.hlsl";
        std::wstring basic2DPSPath = L"Assets/shader/BasicPixelShader.hlsl";
        std::vector<InputLayoutHelper::InputElement> basic2DInputElements =
        {
            { "POSITION", DXGI_FORMAT_R32G32B32_FLOAT },
            { "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT },
        };
        std::vector<DXGI_FORMAT> basic2DRTVFormats = { DXGI_FORMAT_R8G8B8A8_UNORM };

        RootSignatureDesc basic3DRootDesc;
        {
            basic3DRootDesc.params.push_back({ sceneDataParamName, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, D3D12_SHADER_VISIBILITY_ALL });
            basic3DRootDesc.params.push_back({ worldMatParamName, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_ALL });
            basic3DRootDesc.params.push_back({ "mainTex", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL });
            basic3DRootDesc.staticSamplers.push_back({ 0, D3D12_SHADER_VISIBILITY_PIXEL, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_FILTER_ANISOTROPIC });
        }
        std::wstring basic3DVSPath = L"Assets/shader/Basic3DShader.hlsl";
        std::wstring basic3DPSPath = L"Assets/shader/Basic3DShader.hlsl";
        std::vector<InputLayoutHelper::InputElement> basic3DInputElements =
        {
            { "POSITION", DXGI_FORMAT_R32G32B32_FLOAT },
            { "NORMAL", DXGI_FORMAT_R32G32B32_FLOAT },
            { "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT },
        };
        std::vector<DXGI_FORMAT> basic3DRTVFormats = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32G32B32A32_FLOAT };

        RootSignatureDesc postProcessRootDesc;
        {
            postProcessRootDesc.params.push_back({ "srcTex", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL });
            postProcessRootDesc.staticSamplers.push_back({ 0, D3D12_SHADER_VISIBILITY_PIXEL, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_FILTER_ANISOTROPIC });
        }
        std::wstring postProcessVSPath = L"Assets/shader/PostProcess.hlsl";
        std::wstring postProcessPSPath = L"Assets/shader/PostProcess.hlsl";
        std::vector<InputLayoutHelper::InputElement> postProcessInputElements = 
        {
            // SV_VertexIDを使う場合、入力レイアウトは空で良い
        };
        std::vector<DXGI_FORMAT> postProcessRTVFormats = { DXGI_FORMAT_R8G8B8A8_UNORM };

        RootSignatureDesc shadowMapRootDesc;
        {
            shadowMapRootDesc.params.push_back({ sceneDataParamName, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, D3D12_SHADER_VISIBILITY_ALL });
            shadowMapRootDesc.params.push_back({ worldMatParamName, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_ALL });
        }
        std::wstring shadowMapVSPath = L"Assets/shader/Basic3DShadowMap.hlsl";
        std::wstring shadowMapPSPath = L"";
        std::vector<InputLayoutHelper::InputElement> shadowMapInputElements =
        {
            { "POSITION", DXGI_FORMAT_R32G32B32_FLOAT },
            { "NORMAL", DXGI_FORMAT_R32G32B32_FLOAT },
            { "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT },
        };
        std::vector<DXGI_FORMAT> shadowMapRTVFormats = { DXGI_FORMAT_R8G8B8A8_UNORM };

        RootSignatureDesc lightRootDesc;
        {
            lightRootDesc.params.push_back({ sceneDataParamName, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, D3D12_SHADER_VISIBILITY_ALL });
            lightRootDesc.params.push_back({ "albedoTex", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL });
            lightRootDesc.params.push_back({ "normalTex", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL });
            lightRootDesc.params.push_back({ "positionTex", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, D3D12_SHADER_VISIBILITY_PIXEL });
            lightRootDesc.params.push_back({ shadowMapParamName, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, D3D12_SHADER_VISIBILITY_PIXEL });
            lightRootDesc.staticSamplers.push_back({ 0, D3D12_SHADER_VISIBILITY_PIXEL, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_FILTER_ANISOTROPIC });
            lightRootDesc.staticSamplers.push_back({ 1, D3D12_SHADER_VISIBILITY_PIXEL, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR });
        }
        std::wstring lightVSPath = L"Assets/shader/SceneLighting.hlsl";
        std::wstring lightPSPath = L"Assets/shader/SceneLighting.hlsl";
        std::vector<InputLayoutHelper::InputElement> lightInputElements =
        {
            // SV_VertexIDを使う場合、入力レイアウトは空で良い
        };
        std::vector<DXGI_FORMAT> lightRTVFormats = { DXGI_FORMAT_R8G8B8A8_UNORM };

        // 猫のスプライト
        {
		    MaterialDesc materialDesc =
		    {
			    basic2DVSPath,
			    basic2DPSPath,
			    basic2DInputElements,
                basic2DRTVFormats,
			    basic2DRootDesc,
			    BlendPreset::AlphaBlend,
			    RasterizerPreset::CullNode,
			    DepthStencilPreset::DepthDisable,
		    };
		    materialRegistry->Register("cat_sprite_mat", materialDesc);
        }
        // ティーポット
        {
            MaterialDesc materialDesc =
		    {
			    basic3DVSPath,
			    basic3DPSPath,
			    basic3DInputElements,
                basic3DRTVFormats,
			    basic3DRootDesc,
			    BlendPreset::Opaque,
			    RasterizerPreset::CullBack,
			    DepthStencilPreset::DepthEnable,
		    };
		    materialRegistry->Register("teapot_mat", materialDesc);
        }
        // キューブ
        {
            MaterialDesc materialDesc =
            {
                basic3DVSPath,
                basic3DPSPath,
                basic3DInputElements,
                basic3DRTVFormats,
                basic3DRootDesc,
                BlendPreset::Opaque,
                RasterizerPreset::CullBack,
                DepthStencilPreset::DepthEnable,
            };
            materialRegistry->Register("cube_mat", materialDesc);
        }
        // 馬の像
        {
            MaterialDesc materialDesc =
            {
                basic3DVSPath,
                basic3DPSPath,
                basic3DInputElements,
                basic3DRTVFormats,
                basic3DRootDesc,
                BlendPreset::Opaque,
                RasterizerPreset::CullBack,
                DepthStencilPreset::DepthEnable,
            };
            materialRegistry->Register("horse_mat", materialDesc);
        }
        // おもちゃのアヒル
        {
            MaterialDesc materialDesc =
            {
                basic3DVSPath,
                basic3DPSPath,
                basic3DInputElements,
                basic3DRTVFormats,
                basic3DRootDesc,
                BlendPreset::Opaque,
                RasterizerPreset::CullBack,
                DepthStencilPreset::DepthEnable,
            };
            materialRegistry->Register("duck_mat", materialDesc);
        }
        // ラウンジチェア
        {
            MaterialDesc materialDesc =
            {
                basic3DVSPath,
                basic3DPSPath,
                basic3DInputElements,
                basic3DRTVFormats,
                basic3DRootDesc,
                BlendPreset::Opaque,
                RasterizerPreset::CullBack,
                DepthStencilPreset::DepthEnable,
            };
            materialRegistry->Register("chair_mat", materialDesc);
        }
        // 床
        {
            MaterialDesc materialDesc =
            {
                basic3DVSPath,
                basic3DPSPath,
                basic3DInputElements,
                basic3DRTVFormats,
                basic3DRootDesc,
                BlendPreset::Opaque,
                RasterizerPreset::CullBack,
                DepthStencilPreset::DepthEnable,
            };
            materialRegistry->Register("floor_mat", materialDesc);
        }
        // ポストプロセス
        {
            MaterialDesc materialDesc =
            {
                postProcessVSPath,
                postProcessPSPath,
                postProcessInputElements,
                postProcessRTVFormats,
                postProcessRootDesc,
                BlendPreset::Opaque,
                RasterizerPreset::CullNode,
                DepthStencilPreset::DepthDisable,
            };
            materialRegistry->Register("PostProcess", materialDesc);
            auto* mat = materialRegistry->Get("PostProcess");
            mat->SetTexture(GetRootParameterIndex("srcTex", *mat), offscreenRenderTarget->GetColorTexture());
        }
        // シャドウマップ
        {
            MaterialDesc materialDesc =
            {
                shadowMapVSPath,
                shadowMapPSPath,
                shadowMapInputElements,
                shadowMapRTVFormats,
                shadowMapRootDesc,
                BlendPreset::Opaque,
                RasterizerPreset::CullBack,
                DepthStencilPreset::DepthEnable,
            };
            materialRegistry->Register("ShadowMap", materialDesc);
            auto* mat = materialRegistry->Get("ShadowMap");
            mat->SetConstantBuffer(GetRootParameterIndex(sceneDataParamName, *mat), sceneCB);
        }
        // ライト描画
        {
            MaterialDesc materialDesc =
            {
                lightVSPath,
                lightPSPath,
                lightInputElements,
                lightRTVFormats,
                lightRootDesc,
                BlendPreset::Opaque,
                RasterizerPreset::CullNode,
                DepthStencilPreset::DepthDisable,
            };
            materialRegistry->Register("Lighting", materialDesc);
            auto* mat = materialRegistry->Get("Lighting");
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
		// todo: pathで指定されたSpriteを読み込む
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
		return sprite.release();
	}

	Mesh* GraphicsEngine::GetMesh(const std::wstring& path)
	{
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
		return mesh.release();
	}

	Material* GraphicsEngine::GetMaterial(const std::string& name)
	{
		return materialRegistry->Get(name);
	}

	UINT GraphicsEngine::GetRootParameterIndex(const std::string& name, const Material& mat)
	{
		auto& rootSignatureDesc = mat.GetDesc().rootSignatureDesc;
		auto rootSignature = rootSignatureRegistry->GetOrCreate(device, rootSignatureDesc);
		return rootSignature->GetRootIndex(name);
	}

    std::shared_ptr<ConstantBuffer> GraphicsEngine::CreateConstantBuffer(size_t size)
	{
		auto constantBuffer = std::make_shared<ConstantBuffer>();
		constantBuffer->Init(device.Get(), cbv_srv_uav_heap.get(), size);
		return constantBuffer;
	}

    Texture* GraphicsEngine::GetTexture(const std::string& path)
	{
		return textureLoader.GetTexture(path.c_str());
	}

    void GraphicsEngine::InitSceneConstantBuffers(size_t canvasDataSize, size_t sceneDataSize)
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

        auto* shadowMapMat = materialRegistry->Get("ShadowMap");
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

        auto* lightingMat = materialRegistry->Get("Lighting");
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

        auto* postProcessMat = materialRegistry->Get("PostProcess");
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
        MEngine::GUI()->DrawHierarchyWindow(Vector2(0, 0), Vector2((renderTarget->GetWidth()) * 0.25, (renderTarget->GetHeight())), *scene, selectGameObject);
        MEngine::GUI()->DrawSceneViewWindow(Vector2(renderTarget->GetWidth() - renderTarget->GetWidth() * 0.75, 0), Vector2((renderTarget->GetWidth() - 32) * 0.5, (renderTarget->GetHeight() - 32) * 0.5), postProcessRenderTarget->GetColorTexture()->GetSRV());
        MEngine::GUI()->DrawInspectorWindow(Vector2(renderTarget->GetWidth() - renderTarget->GetWidth() * 0.25, 0), Vector2(renderTarget->GetWidth() * 0.25, renderTarget->GetHeight()), selectGameObject);
        MEngine::GUI()->Render(&commandContext, cbv_srv_uav_heap.get());

        graphicsContext.TransitionRenderTargetToPresent(renderTarget);
    }
}
