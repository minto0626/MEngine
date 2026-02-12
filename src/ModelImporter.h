#pragma once
#include <string>
#include <vector>

#include "Mesh/Mesh.h"
#include "Math/Color.h"
#include "AssimpImporter.h"

struct ImportMeshData
{
    std::string name;
    Graphics::MeshData data;
    unsigned int materialIndex;
};

struct ImportMaterialData
{
    std::string name;
    bool useDifuseColor;
    Color diffuseColor;
    bool useSpecularColor;
    Color specularColor;
    float shininess;
    bool useDiffuseTexture;
    std::string diffuseTexturePath;
};

struct ImportModelNode
{
    std::string name;
    std::vector<ImportMeshData> meshs;
    std::vector<ImportModelNode> children;
};

struct ImportModelData
{
    ImportModelNode model;
    std::vector<ImportMaterialData> materials;
};

class ModelImporter
{
private:
	AssimpModelImporter assimpImporter;

public:
	ModelImporter();
	~ModelImporter();

	void Init();

	bool Load(const std::wstring& fileName, ImportModelData& modelData);
};
