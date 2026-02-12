#pragma once
#include <string>
#include <vector>

class AssimpModelImporter
{
private:
	void SetDirectoryAndLoadDll();
    void ParseNode(struct ImportModelNode& modelNode, struct aiNode* node, const struct aiScene* scene);
	void ParseMesh(struct ImportMeshData& dstMesh, const struct aiMesh* srcMesh);
	void ParseMaterial(struct ImportMaterialData& dstMaterial, const struct aiMaterial* srcMaterial, std::string& rootDirectory);

public:
	void Init();

	bool Load(const std::wstring& fileName, struct ImportModelData& modelData);

};
