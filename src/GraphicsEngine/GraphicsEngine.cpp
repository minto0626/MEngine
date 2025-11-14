#include "GraphicsEngine.h"
#include "Shader/Shader.h"
#include "Pipeline/InputLayoutHelper.h"
#include "Pipeline/StateFactory.h"

#include <wrl.h>
#include <Core/d3dx12.h>

using namespace Microsoft::WRL;

namespace Graphics
{
	IDXGIFactory6* GraphicsEngine::CreateDXGIFactory()
	{
		UINT dxgiFactoryFlag = 0;

#if _DEBUG
		ID3D12Debug* debugLayer = nullptr;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer))))
		{
			dxgiFactoryFlag |= DXGI_CREATE_FACTORY_DEBUG;

			debugLayer->EnableDebugLayer();
			debugLayer->Release();
		}
#endif
		IDXGIFactory6* factory;
#if _DEBUG
		CreateDXGIFactory2(dxgiFactoryFlag, IID_PPV_ARGS(&factory));
#else
		CreateDXGIFactory1(IID_PPV_ARGS(&factory));
#endif
		return factory;
	}

	bool GraphicsEngine::Initialize(HWND hwnd, SIZE& windowSize)
	{
		auto* factory = CreateDXGIFactory();
		if (!device.Initialize(factory)) { return false; }
		if (!commandQueue.Initialize(&device, D3D12_COMMAND_LIST_TYPE_DIRECT)) { return false; }
		if (!swapChain.Initialize(hwnd, windowSize, &commandQueue, factory)) { return false; }
		if (!commandContext.Initialize(&device, &commandQueue)) { return false; }
		if (!fence.Initialize(&device)) { return false; }
		factory->Release();

		// モデルインポーター初期化
		modelImporter.Init();

		graphicsContext.device = &device;
		graphicsContext.commandQueue = &commandQueue;
		graphicsContext.commandContext = &commandContext;
		graphicsContext.fence = &fence;

		rtv_heap = std::make_unique<DescriptorHeap>(
			device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 32, false
		);
		DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
		swapChain.Get()->GetDesc1(&swapchainDesc);
		renderTargets.resize(swapchainDesc.BufferCount);
		for (UINT i = 0; i < renderTargets.size(); ++i)
		{
			renderTargets[i] = std::make_unique<RenderTarget>();
			renderTargets[i]->InitFromSwapChain(&device, &swapChain, *rtv_heap, i);
		}

		dsv_heap = std::make_unique<DescriptorHeap>(
			device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 32, false
		);
		depthBuffer = std::make_unique<DepthBuffer>();
		// 深度に32bit使用
		depthBuffer->Init(&device, swapchainDesc.Width, swapchainDesc.Height, DXGI_FORMAT_D32_FLOAT, *dsv_heap);

		cbv_srv_uav_heap = std::make_unique<DescriptorHeap>(
			device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 64
		);
		textureLoader.Init(&graphicsContext);
		scene2DRenderers.clear();
		scene3DRenderers.clear();

		rootSignatureRegistry = std::make_unique<RootSignatureRegistry>();
		materialCache = std::make_unique<MaterialCache>(rootSignatureRegistry.get());
		materialRegistry = std::make_unique<MaterialRegistry>(&device, materialCache.get());

		return true;
	}

	void GraphicsEngine::UnInitialize()
	{
		commandQueue.Signal(fence);
		fence.Wait();
	}

	void GraphicsEngine::LoadContent()
	{
        sceneCB = CreateConstantBuffer(sizeof(SceneConstantBuffer));

		RootSignatureDesc rootDesc1;
		rootDesc1.params.push_back({ worldMatParamName, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX });
		rootDesc1.params.push_back({ "mainTex", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL });
		rootDesc1.staticSamplers.push_back({ 0, D3D12_SHADER_VISIBILITY_PIXEL });
		MaterialDesc materialDesc1 =
		{
			L"Assets/shader/BasicVertexShader.hlsl",
			L"Assets/shader/BasicPixelShader.hlsl",
			{
				{ "POSITION", DXGI_FORMAT_R32G32B32_FLOAT },
				{ "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT },
			},
			rootDesc1,
			BlendPreset::AlphaBlend,
			RasterizerPreset::CullNode,
			DepthStencilPreset::DepthDisable,
		};
		materialRegistry->Register("DefaultMaterial", materialDesc1);

		RootSignatureDesc rootDesc2;
		rootDesc2.params.push_back({ worldMatParamName, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX });
		rootDesc2.params.push_back({ "mainTex", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL });
		rootDesc2.staticSamplers.push_back({ 0, D3D12_SHADER_VISIBILITY_PIXEL });
		MaterialDesc materialDesc2 =
		{
			L"Assets/shader/BasicVertexShader.hlsl",
			L"Assets/shader/BasicPixelShader.hlsl",
			{
				{ "POSITION", DXGI_FORMAT_R32G32B32_FLOAT },
				{ "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT },
			},
			rootDesc2,
			BlendPreset::Opaque,
			RasterizerPreset::CullNode,
			DepthStencilPreset::DepthDisable,
		};
		materialRegistry->Register("NonAlphablendMaterial", materialDesc2);

		RootSignatureDesc rootDesc3;
        rootDesc3.params.push_back({ sceneDataParamName, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, D3D12_SHADER_VISIBILITY_ALL });
		rootDesc3.params.push_back({ worldMatParamName, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_ALL });
		rootDesc3.params.push_back({ "mainTex", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL });
		rootDesc3.staticSamplers.push_back({ 0, D3D12_SHADER_VISIBILITY_PIXEL });
		MaterialDesc materialDesc3 =
		{
			L"Assets/shader/Basic3DShader.hlsl",
			L"Assets/shader/Basic3DShader.hlsl",
			{
				{ "POSITION", DXGI_FORMAT_R32G32B32_FLOAT },
				{ "NORMAL", DXGI_FORMAT_R32G32B32_FLOAT },
				{ "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT },
			},
			rootDesc3,
			BlendPreset::Opaque,
			RasterizerPreset::CullNode,
			DepthStencilPreset::DepthEnable,
		};
		materialRegistry->Register("3DMaterial", materialDesc3);

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
			ComPtr<ID3D12Resource> textureRes = nullptr;
			if (materialDataList.size() > 0 &&
				materialDataList[meshDataSrc.materialIndex].useDiffuseTexture)
			{
				textureRes = textureLoader.GetTextureByPath(materialDataList[meshDataSrc.materialIndex].diffuseTexturePath.c_str());
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

	ConstantBuffer* GraphicsEngine::CreateConstantBuffer(size_t size)
	{
		auto constantBuffer = std::make_unique<ConstantBuffer>();
		constantBuffer->Init(device.Get(), cbv_srv_uav_heap.get(), size);
		return constantBuffer.release();
	}

	Texture* GraphicsEngine::GetTexture(const std::string& path)
	{
		auto texture = std::make_unique<Texture>();
		auto res = textureLoader.GetTextureByPath(path.c_str());
		if (res == nullptr)
		{
			res = textureLoader.GetWhiteTexture();
		}
		texture->Init(device.Get(), cbv_srv_uav_heap.get(), res.Get());
		return texture.release();
	}

	void GraphicsEngine::RegisterSpriteRenderer(SpriteRenderer* spriteRenderer)
	{
		scene2DRenderers.push_back(spriteRenderer);
	}

	void GraphicsEngine::RegisterMeshRenderer(MeshRenderer* meshRenderer)
	{
		scene3DRenderers.push_back(meshRenderer);
	}

	void GraphicsEngine::Render(Camera* camera2D, Camera* camera3D, Light* light)
	{
		auto* commandList = commandContext.GetCommandList();

		UINT backBufferIndex = swapChain.Get()->GetCurrentBackBufferIndex();
		auto* backBuffer = renderTargets[backBufferIndex]->GetResource();

		D3D12_VIEWPORT viewport = CD3DX12_VIEWPORT(backBuffer);
		DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
		swapChain.Get()->GetDesc1(&swapchainDesc);
		D3D12_RECT scissor = CD3DX12_RECT(0, 0, swapchainDesc.Width, swapchainDesc.Height);
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissor);

		commandContext.ResourceBarrier(
			backBuffer,
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET);

		auto rtvHandle = renderTargets[backBufferIndex]->GetCPUHandle();
		auto dsvHandle = depthBuffer->GetCPUHandle();
		commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);
		const float cc[4] = { clearColor.r, clearColor.g, clearColor.b, clearColor.a };
		commandList->ClearRenderTargetView(rtvHandle, cc, 0, nullptr);
		commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		ID3D12DescriptorHeap* const heaps[] = { cbv_srv_uav_heap->GetHeap() };
		commandList->SetDescriptorHeaps(1, heaps);

        // ここでやれるなら、シーンの更新側でやるほうがいいかも
        // シーン共通の定数バッファを更新
        SceneConstantBuffer _sceneCB;
        _sceneCB.camera.viewMatrix = camera3D->GetViewMatrix();
        _sceneCB.camera.projectionMatrix = camera3D->GetProjectionMatrix();
        _sceneCB.camera.cameraPosition = camera3D->GetGameObject()->GetTransform()->GetPos();
        _sceneCB.light.lightDirection = light->GetGameObject()->GetTransform()->GetForward();
        sceneCB->Update(&_sceneCB, sizeof(_sceneCB));

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
                // ここでやるくらいならMaterial::Bind内で行うように変更したほうがいい
                // 3Dシーン用の定数バッファをセット
                auto sceneCBVIndex = GetRootParameterIndex(sceneDataParamName, *currentMaterial);
                commandContext.SetGraphicsRootDescriptorTable(sceneCBVIndex, sceneCB->GetGPUHandle());
			}
			renderer->Draw(&commandContext, camera3D, light);
		}
		for (auto& renderer : scene2DRenderers)
		{
			Material* currentMaterial = renderer->GetMaterial();
			if (currentMaterial && currentMaterial != lastMaterial)
			{
				currentMaterial->Bind(commandContext);
				lastMaterial = currentMaterial;
			}
			renderer->Draw(&commandContext, camera2D, light);
		}

		commandContext.ResourceBarrier(
			backBuffer,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT);

		commandContext.Close();
		ID3D12CommandList* commandLists[] = { commandList };
		commandQueue.Execute(_countof(commandLists), *commandLists);
		commandQueue.Signal(fence);
		fence.Wait();

		commandContext.Reset();

		swapChain.Present();
	}
}
