#pragma once
#include <d3d12.h>
#include <wrl.h>

#include "GfxDevice.h"
#include "GfxCommandQueue.h"

namespace Graphics
{
	class GfxCommandContext
	{
	private:
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> _commandAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList;
		D3D12_COMMAND_LIST_TYPE _type;

	public:
		bool Initialize(GfxDevice* device, GfxCommandQueue* commandQueue);
		ID3D12GraphicsCommandList* GetCommandList() const { return _commandList.Get(); }
		void Reset();
		void Close();

		void SetVertexBuffer(UINT slot, const D3D12_VERTEX_BUFFER_VIEW& vbv);
		void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& ibv);
		void SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY toplogy);
        void DrawInstanced(UINT vertexCount, UINT instanceCount = 1, UINT startVertex = 0, UINT startInstance = 0);
		void DrawIndexedInstanced(UINT indexCount, UINT instanceCount = 1, UINT startIndex = 0, UINT baseVertex = 0, UINT startInstance = 0);
		void SetPipelineState(ID3D12PipelineState* pso);
		void SetRootSignature(ID3D12RootSignature* rootSignature);
		void SetGraphicsRootDescriptorTable(UINT index, D3D12_GPU_DESCRIPTOR_HANDLE handle);
        void SetGraphicsRootConstantBufferView(UINT index, D3D12_GPU_DESCRIPTOR_HANDLE handle);
        void SetGraphicsRoot32BitConstant(UINT index, UINT value, UINT offset);
		void CopyResource(ID3D12Resource* dest, ID3D12Resource* src);
		void ResourceBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to);

	};
}
