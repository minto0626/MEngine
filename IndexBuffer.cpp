#include "IndexBuffer.h"
#include "d3dx12.h"
#include <cassert>

void IndexBuffer::Init(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* commndList,
	const void* indexData,
	UINT indexNum,
	DXGI_FORMAT format)
{
	_indexNum = indexNum;
	_format = format;

	UINT bufferSize = indexNum * (format == DXGI_FORMAT_R16_UINT ? 2 : 4);

	auto def_heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
	auto result = device->CreateCommittedResource(
		&def_heapProp,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(_indexBuffer.ReleaseAndGetAddressOf()));
	if (FAILED(result)) { assert("�C���f�b�N�X�o�b�t�@�[�̍쐬�Ɏ��s!"); return; }

	// �A�b�v���[�h�p�̃q�[�v�ɒ��ԃo�b�t�@�[���쐬
	auto upload_heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	result = device->CreateCommittedResource(
		&upload_heapProp,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_uploadBuffer.ReleaseAndGetAddressOf()));
	if (FAILED(result)) { assert("�A�b�v���[�h�p�̃o�b�t�@�[�̍쐬�Ɏ��s!"); return; }

	// CPU����A�b�v���[�h�p�o�b�t�@�[�Ƀf�[�^���R�s�[
	void* mappedData = nullptr;
	result = _uploadBuffer->Map(0, nullptr, &mappedData);
	if (FAILED(result)) { assert("�A�b�v���[�h�p�̃o�b�t�@�̃}�b�v�Ɏ��s"); return; }
	memcpy(mappedData, indexData, bufferSize);
	_uploadBuffer->Unmap(0, nullptr);

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		_indexBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_COPY_DEST);
	commndList->ResourceBarrier(1, &barrier);

	// �A�b�v���[�h�p�̃o�b�t�@�[����f�t�H���g�p�̃o�b�t�@�[�Ƀf�[�^��]��
	commndList->CopyResource(_indexBuffer.Get(), _uploadBuffer.Get());

	// ���\�[�X��Ԃ𒸓_�o�b�t�@�[�p�ɑJ��
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		_indexBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_INDEX_BUFFER);
	commndList->ResourceBarrier(1, &barrier);

	_indexBuffer->SetName(L"IndexBuffer");

	// �r���[��ݒ�
	_indexBufferView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
	_indexBufferView.SizeInBytes = bufferSize;
	_indexBufferView.Format = format;
}