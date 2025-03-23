#pragma once
#include <vector>
#include <d3d12.h>

namespace InputLayoutHelper
{
	struct InputElement
	{
		const char* semanticName;	// セマンティクス名
		UINT semanticIndex;			// 同じセマンティクス名の時に使うインデックス
		DXGI_FORMAT format;			// データフォーマット
		UINT inputSlot;				// 入力スロットのインデックス
		UINT byteOffset;			// データの場所

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
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,	// 1頂点ごとにレイアウトが入っている
				0	// インスタンシングを利用しない
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