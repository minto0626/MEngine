#include "TextureLoader.h"
#include "d3dx12.h"

using namespace std;
using namespace DirectX;

namespace
{
	// ファイル名から拡張子を得る
	// @param path 対象のパス文字列
	// @return 拡張子
	string GetExtension(const string& path)
	{
		int idx = path.rfind('.');
		return path.substr(idx + 1, path.length() - idx - 1);
	}

	// std::string（マルチバイト文字列）から std::wstring（ワイド文字列）を得る
	// @param str マルチバイト文字列
	// @return 変換されたワイド文字列
	wstring GetWideStringFromString(const string& str)
	{
		// 呼び出し１回目（文字列数を得る）
		auto num1 = MultiByteToWideChar(
			CP_ACP,
			MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
			str.c_str(),
			-1,
			nullptr,
			0);

		wstring wstr;		// stringのwchar_t版
		wstr.resize(num1);	// 得られた文字列数でリサイズ

		// 呼び出し２回目（確保済のwstrに変換文字列をコピー）
		auto num2 = MultiByteToWideChar(
			CP_ACP,
			MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
			str.c_str(),
			-1,
			&wstr[0],
			num1);

		assert(num1 == num2);
		return wstr;
	}
}

void TextureLoader::CreateTextureLoaderTable()
{
	// BMPやPNGなど、Windowsがデフォルトで読める基本的な画像形式
	_loadLamdaTable["sph"]
		= _loadLamdaTable["spa"]
		= _loadLamdaTable["bmp"]
		= _loadLamdaTable["png"]
		= _loadLamdaTable["jpg"]
		= [](const wstring& path, TexMetadata* meta, ScratchImage& image)
		-> HRESULT
		{
			return LoadFromWICFile(path.c_str(), WIC_FLAGS_NONE, meta, image);
		};
	// TGAなどの一部の3Dソフトで使用されているテクスチャファイル形式
	_loadLamdaTable["tga"] = [](const wstring& path, TexMetadata* meta, ScratchImage& image)
		-> HRESULT
		{
			return LoadFromTGAFile(path.c_str(), meta, image);
		};
	// DirectX用の圧縮テクスチャファイル形式
	_loadLamdaTable["dds"] = [](const wstring& path, TexMetadata* meta, ScratchImage& image)
		-> HRESULT
		{
			return LoadFromDDSFile(path.c_str(), DDS_FLAGS_NONE, meta, image);
		};
}

ID3D12Resource* TextureLoader::CreateTextureFromFile(const char* texPath)
{
	auto wtexPath = GetWideStringFromString(texPath);	// テクスチャのファイルパス
	auto ext = GetExtension(texPath);	// 拡張子を取得
	TexMetadata metadata = {};
	ScratchImage scratchImage = {};
	// 読み込める拡張子が存在しない
	if (_loadLamdaTable.find(ext) == _loadLamdaTable.end())
	{
		return nullptr;
	}
	auto result = _loadLamdaTable[ext](wtexPath, &metadata, scratchImage);
	if (FAILED(result))
	{
		return nullptr;
	}

	auto image = scratchImage.GetImage(0, 0, 0);	// 生データ抽出

	// WriteToSubresource で転送する用のヒープ設定
	D3D12_HEAP_PROPERTIES texHeapProp = CD3DX12_HEAP_PROPERTIES(
		D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
		D3D12_MEMORY_POOL_L0);
	// リソースの設定
	D3D12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		metadata.width,
		metadata.height,
		metadata.arraySize,
		metadata.mipLevels);

	// バッファー作成
	ID3D12Resource* texBuffer = nullptr;
	result = _device->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&texBuffer));
	if (FAILED(result))
	{
		return nullptr;
	}

	result = texBuffer->WriteToSubresource(
		0,
		nullptr,			// 全領域コピー
		image->pixels,		// 元データアドレス
		image->rowPitch,	// １ラインサイズ
		image->slicePitch);	// 全サイズ
	if (FAILED(result))
	{
		return nullptr;
	}

	_resourceTable[texPath] = texBuffer;
	return texBuffer;
}

ID3D12Resource* TextureLoader::CreateDefaultTexture(size_t width, size_t height)
{
	auto texHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);
	auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height);

	ID3D12Resource* buffer = nullptr;
	auto result = _device->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&buffer));

	if (FAILED(result))
	{
		assert(SUCCEEDED(result));
		return nullptr;
	}

	return buffer;
}

ID3D12Resource* TextureLoader::CreateWhiteTexture()
{
	ID3D12Resource* whiteBuffer = CreateDefaultTexture(4, 4);

	vector<unsigned char> data(4 * 4 * 4);
	fill(data.begin(), data.end(), 0xff);	// 全て255で埋める
	// データ転送
	auto result = whiteBuffer->WriteToSubresource(
		0,
		nullptr,
		data.data(),
		4 * 4,
		data.size());

	assert(SUCCEEDED(result));

	return whiteBuffer;
}

ID3D12Resource* TextureLoader::CreateBlackTexture()
{
	ID3D12Resource* blackBuffer = CreateDefaultTexture(4, 4);

	vector<unsigned char> data(4 * 4 * 4);
	fill(data.begin(), data.end(), 0x00);	// 全て0で埋める
	// データ転送
	auto result = blackBuffer->WriteToSubresource(
		0,
		nullptr,
		data.data(),
		4 * 4,
		data.size());

	assert(SUCCEEDED(result));

	return blackBuffer;
}

ID3D12Resource* TextureLoader::CreateGrayGradationTexture()
{
	ID3D12Resource* gradBuffer = CreateDefaultTexture(4, 256);

	// 上が白くて下が黒いテクスチャデータを作成
	vector<unsigned int> data(4 * 256);
	auto iterator = data.begin();
	unsigned int c = 0xff;
	for (; iterator != data.end(); iterator += 4)
	{
		auto color = (c << 0xff) | (c << 16) | (c << 8) | c;
		fill(iterator, iterator + 4, color);
		--c;
	}

	auto result = gradBuffer->WriteToSubresource(
		0,
		nullptr,
		data.data(),
		4 * sizeof(unsigned int),
		sizeof(unsigned int) * data.size());

	assert(SUCCEEDED(result));

	return gradBuffer;
}

void TextureLoader::Init(ID3D12Device* device)
{
	_device = device;

	CreateTextureLoaderTable();
	_whiteTexture = CreateWhiteTexture();
	_blackTexture = CreateBlackTexture();
	_gradationTexture = CreateGrayGradationTexture();
}

ComPtr<ID3D12Resource> TextureLoader::GetTextureByPath(const char* texPath)
{
	auto it = _resourceTable.find(texPath);
	if (it != _resourceTable.end()) {
		//テーブルに内にあったらロードするのではなくマップ内の
		//リソースを返す
		return _resourceTable[texPath];
	}
	else {
		return ComPtr<ID3D12Resource>(CreateTextureFromFile(texPath));
	}
}