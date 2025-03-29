#include "TextureLoader.h"
#include "d3dx12.h"

using namespace std;
using namespace DirectX;

namespace
{
	// �t�@�C��������g���q�𓾂�
	// @param path �Ώۂ̃p�X������
	// @return �g���q
	string GetExtension(const string& path)
	{
		int idx = path.rfind('.');
		return path.substr(idx + 1, path.length() - idx - 1);
	}

	// std::string�i�}���`�o�C�g������j���� std::wstring�i���C�h������j�𓾂�
	// @param str �}���`�o�C�g������
	// @return �ϊ����ꂽ���C�h������
	wstring GetWideStringFromString(const string& str)
	{
		// �Ăяo���P��ځi�����񐔂𓾂�j
		auto num1 = MultiByteToWideChar(
			CP_ACP,
			MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
			str.c_str(),
			-1,
			nullptr,
			0);

		wstring wstr;		// string��wchar_t��
		wstr.resize(num1);	// ����ꂽ�����񐔂Ń��T�C�Y

		// �Ăяo���Q��ځi�m�ۍς�wstr�ɕϊ���������R�s�[�j
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
	// BMP��PNG�ȂǁAWindows���f�t�H���g�œǂ߂��{�I�ȉ摜�`��
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
	// TGA�Ȃǂ̈ꕔ��3D�\�t�g�Ŏg�p����Ă���e�N�X�`���t�@�C���`��
	_loadLamdaTable["tga"] = [](const wstring& path, TexMetadata* meta, ScratchImage& image)
		-> HRESULT
		{
			return LoadFromTGAFile(path.c_str(), meta, image);
		};
	// DirectX�p�̈��k�e�N�X�`���t�@�C���`��
	_loadLamdaTable["dds"] = [](const wstring& path, TexMetadata* meta, ScratchImage& image)
		-> HRESULT
		{
			return LoadFromDDSFile(path.c_str(), DDS_FLAGS_NONE, meta, image);
		};
}

ID3D12Resource* TextureLoader::CreateTextureFromFile(const char* texPath)
{
	auto wtexPath = GetWideStringFromString(texPath);	// �e�N�X�`���̃t�@�C���p�X
	auto ext = GetExtension(texPath);	// �g���q���擾
	TexMetadata metadata = {};
	ScratchImage scratchImage = {};
	// �ǂݍ��߂�g���q�����݂��Ȃ�
	if (_loadLamdaTable.find(ext) == _loadLamdaTable.end())
	{
		return nullptr;
	}
	auto result = _loadLamdaTable[ext](wtexPath, &metadata, scratchImage);
	if (FAILED(result))
	{
		return nullptr;
	}

	auto image = scratchImage.GetImage(0, 0, 0);	// ���f�[�^���o

	// WriteToSubresource �œ]������p�̃q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES texHeapProp = CD3DX12_HEAP_PROPERTIES(
		D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
		D3D12_MEMORY_POOL_L0);
	// ���\�[�X�̐ݒ�
	D3D12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		metadata.width,
		metadata.height,
		metadata.arraySize,
		metadata.mipLevels);

	// �o�b�t�@�[�쐬
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
		nullptr,			// �S�̈�R�s�[
		image->pixels,		// ���f�[�^�A�h���X
		image->rowPitch,	// �P���C���T�C�Y
		image->slicePitch);	// �S�T�C�Y
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
	fill(data.begin(), data.end(), 0xff);	// �S��255�Ŗ��߂�
	// �f�[�^�]��
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
	fill(data.begin(), data.end(), 0x00);	// �S��0�Ŗ��߂�
	// �f�[�^�]��
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

	// �オ�����ĉ��������e�N�X�`���f�[�^���쐬
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
		//�e�[�u���ɓ��ɂ������烍�[�h����̂ł͂Ȃ��}�b�v����
		//���\�[�X��Ԃ�
		return _resourceTable[texPath];
	}
	else {
		return ComPtr<ID3D12Resource>(CreateTextureFromFile(texPath));
	}
}