#pragma once
#include <string>
#include <vector>

struct ImportMeshData;
struct ImportMaterialData;

class AssimpModelImporter
{
private:
	void SetDirectoryAndLoadDll();
	void ParseMesh(ImportMeshData& dstMesh, const struct aiMesh* srcMesh);
	void ParseMaterial(ImportMaterialData& dstMaterial, const struct aiMaterial* srcMaterial, std::string& rootDirectory);

public:
	void Init();

	bool Load(
		const std::wstring& fileName,
		std::vector<ImportMeshData>& meshDataList,
		std::vector<ImportMaterialData>& materialDatalist);

};