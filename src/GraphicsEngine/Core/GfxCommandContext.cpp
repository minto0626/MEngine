#include "GfxCommandContext.h"
#include "d3dx12.h"

namespace Graphics
{
	bool GfxCommandContext::Initialize(GfxDevice* device, GfxCommandQueue* commandQueue)
	{
		_type = commandQueue->GetType();

		const auto& _device = device->Get();

		auto result = _device->CreateCommandAllocator(
			_type,
			IID_PPV_ARGS(_commandAllocator.ReleaseAndGetAddressOf()));
		if (FAILED(result))
		{
			return false;
		}
		_commandAllocator->SetName(L"command_allocator");

		result = _device->CreateCommandList(
			0,
			_type,
			_commandAllocator.Get(),
			nullptr,
			IID_PPV_ARGS(_commandList.ReleaseAndGetAddressOf()));
		if (FAILED(result))
		{
			return false;
		}
		_commandList->SetName(L"command_list");

		return true;
	}

	void GfxCommandContext::Reset()
	{
		_commandAllocator->Reset();
		_commandList->Reset(_commandAllocator.Get(), nullptr);
	}

	void GfxCommandContext::Close()
	{
		_commandList->Close();
	}

	void GfxCommandContext::SetVertexBuffer(UINT slot, const D3D12_VERTEX_BUFFER_VIEW& vbv)
	{
		_commandList->IASetVertexBuffers(slot, 1, &vbv);
	}

	void GfxCommandContext::SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& ibv)
	{
		_commandList->IASetIndexBuffer(&ibv);
	}

	void GfxCommandContext::SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY toplogy)
	{
		_commandList->IASetPrimitiveTopology(toplogy);
	}

	void GfxCommandContext::DrawInstanced(UINT vertexCount, UINT instanceCount, UINT startVertex, UINT startInstance)
	{
        _commandList->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);
	}

	void GfxCommandContext::DrawIndexedInstanced(UINT indexCount, UINT instanceCount, UINT startIndex, UINT baseVertex, UINT startInstance)
	{
		_commandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, baseVertex, startInstance);
	}

    void GfxCommandContext::SetDescriptorHeaps(UINT numHeaps, ID3D12DescriptorHeap* const* heaps)
    {
        _commandList->SetDescriptorHeaps(numHeaps, heaps);
    }

	void GfxCommandContext::SetPipelineState(ID3D12PipelineState* pso)
	{
		_commandList->SetPipelineState(pso);
	}

	void GfxCommandContext::SetRootSignature(ID3D12RootSignature* rootSignature)
	{
		_commandList->SetGraphicsRootSignature(rootSignature);
	}

	void GfxCommandContext::SetGraphicsRootDescriptorTable(UINT index, D3D12_GPU_DESCRIPTOR_HANDLE handle)
	{
		_commandList->SetGraphicsRootDescriptorTable(index, handle);
	}

    void GfxCommandContext::SetGraphicsRootConstantBufferView(UINT index, D3D12_GPU_DESCRIPTOR_HANDLE handle)
    {
        _commandList->SetGraphicsRootConstantBufferView(index, handle.ptr);
    }

    void GfxCommandContext::SetGraphicsRoot32BitConstant(UINT index, UINT value, UINT offset)
    {
        _commandList->SetGraphicsRoot32BitConstant(index, value, offset);
    }

	void GfxCommandContext::CopyResource(ID3D12Resource* dest, ID3D12Resource* src)
	{
		_commandList->CopyResource(dest, src);
	}

	void GfxCommandContext::ResourceBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to)
	{
		auto desc = CD3DX12_RESOURCE_BARRIER::Transition(resource, from, to);
		_commandList->ResourceBarrier(1, &desc);
	}

    void GfxCommandContext::SetRenderTargets(UINT numRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandles, bool useDSV, const D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle)
    {
        _commandList->OMSetRenderTargets(numRTVs, rtvHandles, false, useDSV ? dsvHandle : nullptr);
    }

    void GfxCommandContext::ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, const float color[4])
    {
        _commandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
    }

    void GfxCommandContext::ClearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle, D3D12_CLEAR_FLAGS flags, float depth, UINT8 stencil)
    {
        _commandList->ClearDepthStencilView(dsvHandle, flags, depth, stencil, 0, nullptr);
    }

    void GfxCommandContext::SetViewport(D3D12_VIEWPORT& viewport)
    {
        _commandList->RSSetViewports(1, &viewport);
    }

    void GfxCommandContext::SetScissorRect(D3D12_RECT& scissor)
    {
        _commandList->RSSetScissorRects(1, &scissor);
    }
}
