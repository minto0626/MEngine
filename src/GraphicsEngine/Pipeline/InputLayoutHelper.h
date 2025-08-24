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

		bool operator ==(const InputElement& other) const 
		{
			return
				semanticName == other.semanticName &&
				semanticIndex == other.semanticIndex &&
				format == other.format &&
				inputSlot == other.inputSlot &&
				byteOffset == other.byteOffset;
		}
	};

	std::vector<D3D12_INPUT_ELEMENT_DESC> CreateInputLayout(const std::vector<InputElement>& elements);
}

namespace std
{
	template<>
	struct hash<InputLayoutHelper::InputElement>
	{
		size_t operator ()(const InputLayoutHelper::InputElement& e) const
		{
			size_t h = 0;
			h ^= std::hash<const char*>{}(e.semanticName) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= std::hash<UINT>{}(e.semanticIndex) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= std::hash<DXGI_FORMAT>{}(e.format) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= std::hash<UINT>{}(e.inputSlot) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= std::hash<UINT>{}(e.byteOffset) + 0x9e3779b9 + (h << 6) + (h >> 2);
			return h;
		}
	};
}