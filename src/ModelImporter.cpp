#include "ModelImporter.h"

ModelImporter::ModelImporter()
{
}

ModelImporter::~ModelImporter()
{
}

void ModelImporter::Init()
{
	assimpImporter.Init();
}

bool ModelImporter::Load(
	const std::wstring& fileName,
	std::vector<ImportMeshData>& meshDataList,
	std::vector<ImportMaterialData>& materialDatalist)
{
	return assimpImporter.Load(fileName, meshDataList, materialDatalist);
}