#include "GraphicsEngine.h"
#include <wrl.h>
#include <d3dx12.h>

#include "Shader.h"
#include "InputLayoutHelper.h"

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

		camera2D.Init(windowSize.cx, windowSize.cy);

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

		// メッシュ１
		mesh = std::make_unique<Mesh>();
		mesh->Initialize(device.Get(), commandContext);

		material = std::make_unique<Material>();
		Shader vs, ps;
		vs.LoadVS(L"Assets/shader/BasicVertexShader.hlsl", "vs");
		ps.LoadPS(L"Assets/shader/BasicPixelShader.hlsl", "ps");

		rootSignature = std::make_unique<RootSignature>();
		rootSignature->AddDescriptorTable(worldMatParamName, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX, D3D12_DESCRIPTOR_RANGE_TYPE_CBV);
		rootSignature->AddDescriptorTable(mainTexParamName, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
		rootSignature->AddStaticSampler(0, D3D12_SHADER_VISIBILITY_PIXEL);
		rootSignature->Build(device.Get());

		pipelineState = std::make_unique<PipelineState>();
		auto inputLayout = InputLayoutHelper::CreateInputLayout(
			{
				{ "POSITION", DXGI_FORMAT_R32G32B32_FLOAT },
				{ "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT },
			}
			);

		D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc = {};
		pipelineStateDesc.pRootSignature = rootSignature->Get();
		pipelineStateDesc.VS = vs.GetBytecode();
		pipelineStateDesc.PS = ps.GetBytecode();
		pipelineStateDesc.InputLayout = { inputLayout.data(), static_cast<UINT>(inputLayout.size()) };
		pipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
		pipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		pipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;  // カリングしない
		pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		pipelineStateDesc.BlendState.AlphaToCoverageEnable = true;  // アルファテストする
		pipelineStateDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;    // カットなし
		pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;   // 三角形で描画
		pipelineStateDesc.NumRenderTargets = 1; // レンダーターゲットは一つ
		pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		pipelineStateDesc.SampleDesc.Count = 1;
		pipelineStateDesc.SampleDesc.Quality = 0;
		pipelineState->Init(device.Get(), pipelineStateDesc);

		material->SetPipelineState(pipelineState.get());

		const char* texfilePath = "Assets/texture/free_ei.png";
		texture = std::make_unique<Texture>();
		texture->Init(device.Get(), cbv_srv_uav_heap.get(), textureLoader.GetTextureByPath(texfilePath).Get());
		material->SetTexture(rootSignature->GetRootIndex(mainTexParamName), texture.get());

		constantBuffer = std::make_unique<ConstantBuffer>();
		transform.SetPos({640, 360, 0});
		auto world = transform.GetWorldMatrix();
		constantBuffer->Init(device.Get(), cbv_srv_uav_heap.get(), sizeof(world));
		material->SetConstantBuffer(rootSignature->GetRootIndex(worldMatParamName), constantBuffer.get());
		world *= cameraWorld;
		material->UploadConstantBuffer(rootSignature->GetRootIndex(worldMatParamName), &world, sizeof(world));

		meshRenderer = std::make_unique<MeshRenderer>(mesh.get(), material.get());
		sceneRenderers.push_back(meshRenderer.get());

		// メッシュ２
		mesh2 = std::make_unique<Mesh>();
		mesh2->Initialize(device.Get(), commandContext);

		material2 = std::make_unique<Material>();

		rootSignature2 = std::make_unique<RootSignature>();
		rootSignature2->AddDescriptorTable(worldMatParamName, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX, D3D12_DESCRIPTOR_RANGE_TYPE_CBV);
		rootSignature2->AddDescriptorTable(mainTexParamName, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
		rootSignature2->AddStaticSampler(0, D3D12_SHADER_VISIBILITY_PIXEL);
		rootSignature2->Build(device.Get());
		
		pipelineState2 = std::make_unique<PipelineState>();
		pipelineStateDesc.pRootSignature = rootSignature2->Get();
		pipelineState2->Init(device.Get(), pipelineStateDesc);

		material2->SetPipelineState(pipelineState2.get());

		const char* texfilePath2 = "Assets/texture/free_brachiosaurus.png";
		texture2 = std::make_unique<Texture>();
		texture2->Init(device.Get(), cbv_srv_uav_heap.get(), textureLoader.GetTextureByPath(texfilePath2).Get());
		material2->SetTexture(rootSignature2->GetRootIndex(mainTexParamName), texture2.get());

		constantBuffer2 = std::make_unique<ConstantBuffer>();
		transform2.SetPos({ 1080, 360, 0 });
		auto world2 = transform2.GetWorldMatrix();
		constantBuffer2->Init(device.Get(), cbv_srv_uav_heap.get(), sizeof(world2));
		material2->SetConstantBuffer(rootSignature2->GetRootIndex(worldMatParamName), constantBuffer2.get());
		world2 *= cameraWorld;
		material2->UploadConstantBuffer(rootSignature2->GetRootIndex(worldMatParamName), &world2, sizeof(world2));

		meshRenderer2 = std::make_unique<MeshRenderer>(mesh2.get(), material2.get());
		sceneRenderers.push_back(meshRenderer2.get());

		// メッシュ３
		mesh3 = std::make_unique<Mesh>();
		mesh3->Initialize(device.Get(), commandContext);

		material3 = std::make_unique<Material>();

		rootSignature3 = std::make_unique<RootSignature>();
		rootSignature3->AddDescriptorTable(worldMatParamName, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX, D3D12_DESCRIPTOR_RANGE_TYPE_CBV);
		rootSignature3->AddDescriptorTable(mainTexParamName, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
		rootSignature3->AddStaticSampler(0, D3D12_SHADER_VISIBILITY_PIXEL);
		rootSignature3->Build(device.Get());

		pipelineState3 = std::make_unique<PipelineState>();
		pipelineStateDesc.pRootSignature = rootSignature3->Get();
		pipelineState3->Init(device.Get(), pipelineStateDesc);

		material3->SetPipelineState(pipelineState3.get());

		const char* texfilePath3 = "Assets/texture/free_woman_veterinarian.png";
		texture3 = std::make_unique<Texture>();
		texture3->Init(device.Get(), cbv_srv_uav_heap.get(), textureLoader.GetTextureByPath(texfilePath3).Get());
		material3->SetTexture(rootSignature3->GetRootIndex(mainTexParamName), texture3.get());

		constantBuffer3 = std::make_unique<ConstantBuffer>();
		transform3.SetPos({ 300, 360, 0 });
		auto world3 = transform3.GetWorldMatrix();
		constantBuffer3->Init(device.Get(), cbv_srv_uav_heap.get(), sizeof(world3));
		material3->SetConstantBuffer(rootSignature3->GetRootIndex(worldMatParamName), constantBuffer3.get());
		world3 *= cameraWorld;
		material3->UploadConstantBuffer(rootSignature3->GetRootIndex(worldMatParamName), &world3, sizeof(world3));

		meshRenderer3 = std::make_unique<MeshRenderer>(mesh3.get(), material3.get());
		sceneRenderers.push_back(meshRenderer3.get());
	}

	void GraphicsEngine::Render()
	{
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

		for (auto& renderer : sceneRenderers)
		{
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