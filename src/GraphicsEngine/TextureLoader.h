#pragma once
#include <d3d12.h>
#include <map>
#include <string>
#include <functional>
#include <memory>
#include <DirectXTex.h>
#include <wrl.h>

#include "Core/GraphicsContext.h"
#include "Resources/Texture.h"

class TextureLoader
{
private:

	Graphics::GraphicsContext* _graphicsContext;
    class DescriptorHeap* _cbvSrvUavHeap;

	// ロード用テーブル
	using LoadLamda_t = std::function<HRESULT(const std::wstring& path, DirectX::TexMetadata*, DirectX::ScratchImage&)>;
	std::map<std::string, LoadLamda_t> _loadLamdaTable;

	// ファイル名パスとリソースのマップテーブル
    std::map<std::string, std::shared_ptr<Texture>> _textures;

	// テクスチャローダテーブルの作成
	void CreateTextureLoaderTable();

	// 指定テクスチャのロード
    std::shared_ptr<Texture> CreateTexture(const char* texPath);

public:
	void Init(Graphics::GraphicsContext* graphicsContext, class DescriptorHeap* descHeap);
    Texture* GetTexture(const char* texPath);

};
