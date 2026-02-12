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

bool ModelImporter::Load(const std::wstring& fileName, ImportModelData& modelData)
{
	return assimpImporter.Load(fileName, modelData);
}
