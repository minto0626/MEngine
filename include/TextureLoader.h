#pragma once
#include <d3d12.h>
#include <map>
#include <string>
#include <functional>
#include <DirectXTex.h>
#include <wrl.h>

class TextureLoader
{
private:

	ID3D12Device* _device;

	// ロード用テーブル
	using LoadLamda_t = std::function<HRESULT(const std::wstring& path, DirectX::TexMetadata*, DirectX::ScratchImage&)>;
	std::map<std::string, LoadLamda_t> _loadLamdaTable;

	// ファイル名パスとリソースのマップテーブル
	std::map<std::string, ID3D12Resource*> _resourceTable;

	// テクスチャローダテーブルの作成
	void CreateTextureLoaderTable();

	// 指定テクスチャのロード
	ID3D12Resource* CreateTextureFromFile(const char* texPath);

	// デフォルトテクスチャ
	Microsoft::WRL::ComPtr<ID3D12Resource> _whiteTexture;
	Microsoft::WRL::ComPtr<ID3D12Resource> _blackTexture;
	Microsoft::WRL::ComPtr<ID3D12Resource> _gradationTexture;
	ID3D12Resource* CreateDefaultTexture(size_t width, size_t height);
	ID3D12Resource* CreateWhiteTexture();
	ID3D12Resource* CreateBlackTexture();
	ID3D12Resource* CreateGrayGradationTexture();

public:
	void Init(ID3D12Device* device);
	Microsoft::WRL::ComPtr<ID3D12Resource> GetTextureByPath(const char* texPath);

	Microsoft::WRL::ComPtr<ID3D12Resource> GetWhiteTexture() const { return _whiteTexture.Get(); }
	Microsoft::WRL::ComPtr<ID3D12Resource> GetBlackTexture() const { return _blackTexture.Get(); }
	Microsoft::WRL::ComPtr<ID3D12Resource> GetGradationTexture() const { return _gradationTexture.Get(); }

};