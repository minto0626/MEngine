#pragma once
#include <string>
#include <vector>

#include "Mesh/Mesh.h"
#include "Math/Color.h"
#include "AssimpImporter.h"

struct ImportMeshData
{
	std::vector<Graphics::MeshVertex> vertices;
	std::vector<uint32_t> indices;
	unsigned int materialIndex;
};

struct ImportMaterialData
{
	bool useDifuseColor;
	Color diffuseColor;
	bool useSpecularColor;
	Color specularColor;
	float shininess;
	bool useDiffuseTexture;
	std::string diffuseTexturePath;
};

class ModelImporter
{
private:
	AssimpModelImporter assimpImporter;

public:
	ModelImporter();
	~ModelImporter();

	void Init();

	bool Load(
		const std::wstring& fileName,
		std::vector<ImportMeshData>& meshDataList,
		std::vector<ImportMaterialData>& materialDatalist);
};