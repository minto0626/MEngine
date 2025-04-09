#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "d3dx12.h"

class Shader
{
private:
	Microsoft::WRL::ComPtr<ID3DBlob> _blob;

	void Load(const wchar_t* filePath, const char* entryPoint, const char* shaderModel);
	bool CheckShaderCompileResult(HRESULT result, const wchar_t* filePath, ID3DBlob* errorBlob);

public:
	void LoadVS(const wchar_t* filePath, const char* entryPoint);
	void LoadPS(const wchar_t* filePath, const char* entryPoint);
	void LoadCSO(const wchar_t* csoFilePath);

	D3D12_SHADER_BYTECODE GetBytecode() const { return CD3DX12_SHADER_BYTECODE(_blob.Get()); }
};