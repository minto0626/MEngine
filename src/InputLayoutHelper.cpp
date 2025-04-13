#include "InputLayoutHelper.h"

std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayoutHelper::CreateInputLayout(const std::initializer_list<InputElement>& elements)
{
	std::vector<D3D12_INPUT_ELEMENT_DESC> layout;
	layout.reserve(elements.size());
	for (const auto& element : elements)
	{
		layout.push_back(element.ToDesc());
	}
	return layout;
}