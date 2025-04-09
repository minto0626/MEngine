#include "Shader.h"
#include <d3dcompiler.h>
#include <cassert>
#include <fstream>
#include "Debug.h"

using namespace Microsoft::WRL;

namespace
{
	const char* vs_shaderModelName = "vs_5_0";
	const char* ps_shaderModelName = "ps_5_0";
}

void Shader::Load(const wchar_t* filePath, const char* entryPoint, const char* shaderModel)
{
	ComPtr<ID3DBlob> errorBlob;
	UINT compileFlags = 0;
#if _DEBUG
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	auto result = D3DCompileFromFile(
		filePath,
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint, shaderModel,
		compileFlags, 0,
		&_blob, &errorBlob);

	if (!CheckShaderCompileResult(result, filePath, errorBlob.Get()))
	{
		assert(0);
		return;
	}
}

bool Shader::CheckShaderCompileResult(HRESULT result, const wchar_t* filePath, ID3DBlob* errorBlob)
{
	if (SUCCEEDED(result))
	{
		return true;
	}

	if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
	{
		std::wstring errorMes = L"指定されたシェーダーファイルが見つかりませんでした。";
		errorMes += filePath;
		Debug::ShowErrorMessageBox(errorMes, L"エラー");
	}
	else
	{
		std::string errorMes;
		errorMes.resize(errorBlob->GetBufferSize());
		copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errorMes.begin());
		errorMes += "\n";
		Debug::ShowErrorMessageBox(errorMes, "シェーダーコンパイルエラー");
	}
	return false;
}

void Shader::LoadVS(const wchar_t* filePath, const char* entryPoint)
{
	Load(filePath, entryPoint, vs_shaderModelName);
}

void Shader::LoadPS(const wchar_t* filePath, const char* entryPoint)
{
	Load(filePath, entryPoint, ps_shaderModelName);
}

void Shader::LoadCSO(const wchar_t* csoFilePath)
{
	// プリコンパイルされたシェーダーオブジェクトを読み込む

	std::ifstream file(csoFilePath, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		assert(0 && "csoファイルの読み込みに失敗!");
		return;
	}

	// ファイルサイズを取得し、バッファーを確保
	size_t fileSize = static_cast<size_t>(file.tellg());
	file.seekg(0, std::ios::beg);

	auto result = D3DCreateBlob(fileSize, &_blob);
	if (FAILED(result))
	{
		assert(0 && "シェーダーオブジェクトの作成に失敗!");
		return;
	}

	// ファイルの内容をBlobに読み込む
	file.read(static_cast<char*>(_blob->GetBufferPointer()), fileSize);
	file.close();
}