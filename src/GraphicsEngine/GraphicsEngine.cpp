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
		rtvHandles.resize(swapchainDesc.BufferCount);
		for (UINT i = 0; i < renderTargets.size(); ++i)
		{
			swapChain.Get()->GetBuffer(i, IID_PPV_ARGS(renderTargets[i].ReleaseAndGetAddressOf()));
			auto handle = rtv_heap->Allocate();
			rtvHandles[i] = handle;
			device.Get()->CreateRenderTargetView(renderTargets[i].Get(), nullptr, handle.cpuHandle);
		}

		cbv_srv_uav_heap = std::make_unique<DescriptorHeap>(
			device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 64
		);
		textureLoader.Init(&graphicsContext);
		sceneRenderers.clear();

		rootSignatureRegistry = std::make_unique<RootSignatureRegistry>();
		materialCache = std::make_unique<MaterialCache>(rootSignatureRegistry.get());
		materialRegistry = std::make_unique<MaterialRegistry>(&device, materialCache.get());

		camera2D.Init(Camera::ProjectionType::Ortho, windowSize.cx, windowSize.cy);
		camera2D.SetPos({ 0.0f, 0.0f, 0.0f });

		camera3D.Init(Camera::ProjectionType::Perspective, windowSize.cx, windowSize.cy);
		camera3D.SetPos({ 0.0f, 0.0f, -5.0f });
		camera3D.SetTarget({ 0.0f, 0.0f, 0.0f });

		return true;
	}

	void GraphicsEngine::UnInitialize()
	{
		commandQueue.Signal(fence);
		fence.Wait();
	}

	void GraphicsEngine::LoadContent()
	{
		auto cameraWorld = camera2D.GetViewMatrix();

		RootSignatureDesc rootDesc1;
		rootDesc1.params.push_back({ worldMatParamName, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX });
		rootDesc1.params.push_back({ mainTexParamName, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL });
		rootDesc1.staticSamplers.push_back({ 0, D3D12_SHADER_VISIBILITY_PIXEL });
		MaterialDesc materialDesc1 =
		{
			L"Assets/shader/BasicVertexShader.hlsl",
			L"Assets/shader/BasicPixelShader.hlsl",
			rootDesc1,
			BlendPreset::AlphaBlend,
			RasterizerPreset::CullNode,
			DepthStencilPreset::DepthDisable,
		};
		materialRegistry->Register("DefaultMaterial", materialDesc1);

		RootSignatureDesc rootDesc2;
		rootDesc2.params.push_back({ worldMatParamName, D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX });
		rootDesc2.params.push_back({ mainTexParamName, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL });
		rootDesc2.staticSamplers.push_back({ 0, D3D12_SHADER_VISIBILITY_PIXEL });
		MaterialDesc materialDesc2 =
		{
			L"Assets/shader/BasicVertexShader.hlsl",
			L"Assets/shader/BasicPixelShader.hlsl",
			rootDesc2,
			BlendPreset::Opaque,
			RasterizerPreset::CullNode,
			DepthStencilPreset::DepthDisable,
		};
		materialRegistry->Register("NonAlphablendMaterial", materialDesc2);

		// メッシュ１
		{
			MeshData meshData;
			auto halfW = 500.0f * 0.5f;
			auto halfH = 500.0f * 0.5f;
			meshData.vertices = {
				{{ -halfW,  halfH, 0.0f }, { 0.0f, 1.0f }},   // 左下
				{{ -halfW, -halfH, 0.0f }, { 0.0f, 0.0f }},   // 左上
				{{  halfW,  halfH, 0.0f }, { 1.0f, 1.0f }},   // 右下
				{{  halfW, -halfH, 0.0f }, { 1.0f, 0.0f }},   // 右上
			};
			meshData.indices = {
				0, 1, 2,
				2, 1, 3,
			};

			mesh = std::make_unique<Mesh>();
			mesh->Initialize(device.Get(), commandContext, meshData);

			auto rootSignature = rootSignatureRegistry->GetOrCreate(device, rootDesc1);

			auto material = materialRegistry->Get("DefaultMaterial");

			const char* texfilePath = "Assets/texture/free_ei.png";
			texture = std::make_unique<Texture>();
			texture->Init(device.Get(), cbv_srv_uav_heap.get(), textureLoader.GetTextureByPath(texfilePath).Get());
			material->SetTexture(rootSignature->GetRootIndex(mainTexParamName), texture.get());

			constantBuffer = std::make_unique<ConstantBuffer>();
			transform.SetPos({640, 360, 0});
			auto world = transform.GetWorldMatrix();
			constantBuffer->Init(device.Get(), cbv_srv_uav_heap.get(), sizeof(world));
			world *= camera2D.GetViewProjectionMatrix();
			constantBuffer->Update(&world, sizeof(world));

			meshRenderer = std::make_unique<MeshRenderer>(mesh.get(), material, rootSignature->GetRootIndex(worldMatParamName), constantBuffer.get());
			sceneRenderers.push_back(meshRenderer.get());
		}

		// メッシュ２
		{
			MeshData meshData;
			auto halfW = 200.0f * 0.5f;
			auto halfH = 200.0f * 0.5f;
			meshData.vertices = {
				{{ -halfW,  halfH, 0.0f }, { 0.0f, 1.0f }},   // 左下
				{{ -halfW, -halfH, 0.0f }, { 0.0f, 0.0f }},   // 左上
				{{  halfW,  halfH, 0.0f }, { 1.0f, 1.0f }},   // 右下
				{{  halfW, -halfH, 0.0f }, { 1.0f, 0.0f }},   // 右上
			};
			meshData.indices = {
				0, 1, 2,
				2, 1, 3,
			};

			mesh2 = std::make_unique<Mesh>();
			mesh2->Initialize(device.Get(), commandContext, meshData);

			auto rootSignature = rootSignatureRegistry->GetOrCreate(device, rootDesc1);

			auto material2 = materialRegistry->Get("DefaultMaterial");

			const char* texfilePath2 = "Assets/texture/free_brachiosaurus.png";
			texture2 = std::make_unique<Texture>();
			texture2->Init(device.Get(), cbv_srv_uav_heap.get(), textureLoader.GetTextureByPath(texfilePath2).Get());
			material2->SetTexture(rootSignature->GetRootIndex(mainTexParamName), texture2.get());

			constantBuffer2 = std::make_unique<ConstantBuffer>();
			transform2.SetPos({ 850, 360, 0 });
			auto world2 = transform2.GetWorldMatrix();
			constantBuffer2->Init(device.Get(), cbv_srv_uav_heap.get(), sizeof(world2));
			world2 *= camera2D.GetViewProjectionMatrix();
			constantBuffer2->Update(&world2, sizeof(world2));

			meshRenderer2 = std::make_unique<MeshRenderer>(mesh2.get(), material2, rootSignature->GetRootIndex(worldMatParamName), constantBuffer2.get());
			sceneRenderers.push_back(meshRenderer2.get());
		}

		// メッシュ３
		{
			MeshData meshData;
			auto halfW = 300.0f * 0.5f;
			auto halfH = 600.0f * 0.5f;
			meshData.vertices = {
				{{ -halfW,  halfH, 0.0f }, { 0.0f, 1.0f }},   // 左下
				{{ -halfW, -halfH, 0.0f }, { 0.0f, 0.0f }},   // 左上
				{{  halfW,  halfH, 0.0f }, { 1.0f, 1.0f }},   // 右下
				{{  halfW, -halfH, 0.0f }, { 1.0f, 0.0f }},   // 右上
			};
			meshData.indices = {
				0, 1, 2,
				2, 1, 3,
			};

			mesh3 = std::make_unique<Mesh>();
			mesh3->Initialize(device.Get(), commandContext, meshData);

			auto rootSignature = rootSignatureRegistry->GetOrCreate(device, rootDesc2);

			auto material3 = materialRegistry->Get("NonAlphablendMaterial");

			const char* texfilePath3 = "Assets/texture/free_woman_veterinarian.png";
			texture3 = std::make_unique<Texture>();
			texture3->Init(device.Get(), cbv_srv_uav_heap.get(), textureLoader.GetTextureByPath(texfilePath3).Get());
			material3->SetTexture(rootSignature->GetRootIndex(mainTexParamName), texture3.get());

			constantBuffer3 = std::make_unique<ConstantBuffer>();
			transform3.SetPos({ 450, 360, 0 });
			auto world3 = transform3.GetWorldMatrix();
			constantBuffer3->Init(device.Get(), cbv_srv_uav_heap.get(), sizeof(world3));
			world3 *= camera2D.GetViewProjectionMatrix();
			constantBuffer3->Update(&world3, sizeof(world3));

			meshRenderer3 = std::make_unique<MeshRenderer>(mesh3.get(), material3, rootSignature->GetRootIndex(worldMatParamName), constantBuffer3.get());
			sceneRenderers.push_back(meshRenderer3.get());
		}
	}

	void GraphicsEngine::Render()
	{
		camera2D.Update();
		camera3D.Update();

		auto* commandList = commandContext.GetCommandList();

		UINT backBufferIndex = swapChain.Get()->GetCurrentBackBufferIndex();
		auto* backBuffer = renderTargets[backBufferIndex].Get();

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

		auto rtvHandle = rtvHandles[backBufferIndex].cpuHandle;
		commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);
		const float cc[4] = { clearColor.r, clearColor.g, clearColor.b, clearColor.a };
		commandList->ClearRenderTargetView(rtvHandle, cc, 0, nullptr);

		ID3D12DescriptorHeap* const heaps[] = { cbv_srv_uav_heap->GetHeap() };
		commandList->SetDescriptorHeaps(1, heaps);

		// 同じマテリアルでソートする。パイプラインの切り替えを最小限に抑えるため。
		std::sort(sceneRenderers.begin(), sceneRenderers.end(),
			[](const Renderer* a, const Renderer* b)
			{
				Material* materialA = a->GetMaterial();
				Material* materialB = b->GetMaterial();
				if (!materialA || !materialB) { return materialA < materialB; }
				return materialA->GetInstanceID() < materialB->GetInstanceID();
			});

		Material* lastMaterial = nullptr;

		for (auto& renderer : sceneRenderers)
		{
			Material* currentMaterial = renderer->GetMaterial();
			if (currentMaterial && currentMaterial != lastMaterial)
			{
				currentMaterial->Bind(commandContext);
				lastMaterial = currentMaterial;
			}
			renderer->Draw(&commandContext);
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