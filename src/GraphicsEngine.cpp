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
		renderer = std::make_unique<Renderer>();

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
		mesh = std::make_unique<Mesh>();
		mesh->Initialize(device.Get(), commandContext);

		material = std::make_unique<Material>();
		Shader vs, ps;
		vs.LoadVS(L"Assets/shader/BasicVertexShader.hlsl", "vs");
		ps.LoadPS(L"Assets/shader/BasicPixelShader.hlsl", "ps");

		rootSignature = std::make_unique<RootSignature>();
		rootSignature->AddDescriptorTable(1, 0, D3D12_SHADER_VISIBILITY_VERTEX, D3D12_DESCRIPTOR_RANGE_TYPE_CBV);
		rootSignature->AddDescriptorTable(1, 0, D3D12_SHADER_VISIBILITY_PIXEL, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
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
		material->SetTexture(texture.get());

		constantBuffer = std::make_unique<ConstantBuffer>();
		transform.SetPos({640, 360, 0});
		auto world = transform.GetWorldMatrix();
		constantBuffer->Init(device.Get(), cbv_srv_uav_heap.get(), sizeof(world));
		material->SetConstantBuffer(constantBuffer.get());
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
		commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		ID3D12DescriptorHeap* const heaps[] = { cbv_srv_uav_heap->GetHeap() };
		commandList->SetDescriptorHeaps(1, heaps);
		auto world = transform.GetWorldMatrix();
		world *= camera2D.GetViewMatrix();
		material->UploadConstantBuffer(&world, sizeof(world));
		renderer->Draw(&commandContext, mesh.get(), material.get());

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