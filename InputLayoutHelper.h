#pragma once
#include <vector>
#include <d3d12.h>

namespace InputLayoutHelper
{
	struct InputElement
	{
		const char* semanticName;	// �Z�}���e�B�N�X��
		UINT semanticIndex;			// �����Z�}���e�B�N�X���̎��Ɏg���C���f�b�N�X
		DXGI_FORMAT format;			// �f�[�^�t�H�[�}�b�g
		UINT inputSlot;				// ���̓X���b�g�̃C���f�b�N�X
		UINT byteOffset;			// �f�[�^�̏ꏊ

		InputElement(const char* semanticName, DXGI_FORMAT format) :
			semanticName(semanticName),
			semanticIndex(0),
			format(format),
			inputSlot(0),
			byteOffset(D3D12_APPEND_ALIGNED_ELEMENT)
		{}

		D3D12_INPUT_ELEMENT_DESC ToDesc() const
		{
			return
			{
				semanticName,
				semanticIndex,
				format,
				inputSlot,
				byteOffset,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,	// 1���_���ƂɃ��C�A�E�g�������Ă���
				0	// �C���X�^���V���O�𗘗p���Ȃ�
			};
		}
	};

	std::vector<D3D12_INPUT_ELEMENT_DESC> CreateInputLayout(
		const std::initializer_list<InputElement>& elements)
	{
		std::vector<D3D12_INPUT_ELEMENT_DESC> layout;
		layout.reserve(elements.size());
		for (const auto& element : elements)
		{
			layout.push_back(element.ToDesc());
		}
		return layout;
	}
}