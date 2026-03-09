#include "ModelImporter.h"

#include "MEngine.h"
#include <filesystem>
#include <fstream>

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

void writeModelNode(std::ofstream& modelBinaly, ImportModelNode& node)
{
    size_t nameSize = node.name.size();
    modelBinaly.write(reinterpret_cast<const char*>(&nameSize), sizeof(nameSize));
    modelBinaly.write(node.name.c_str(), nameSize);
    unsigned int meshNum = static_cast<unsigned int>(node.meshs.size());
    modelBinaly.write(reinterpret_cast<const char*>(&meshNum), sizeof(meshNum));
    for (const auto& mesh : node.meshs)
    {
        unsigned int vertexNum = static_cast<unsigned int>(mesh.data.vertices.size());
        modelBinaly.write(reinterpret_cast<const char*>(&vertexNum), sizeof(vertexNum));
        modelBinaly.write(reinterpret_cast<const char*>(mesh.data.vertices.data()), vertexNum * sizeof(mesh.data.vertices[0]));
        unsigned int indexNum = static_cast<unsigned int>(mesh.data.indices.size());
        modelBinaly.write(reinterpret_cast<const char*>(&indexNum), sizeof(indexNum));
        modelBinaly.write(reinterpret_cast<const char*>(mesh.data.indices.data()), indexNum * sizeof(mesh.data.indices[0]));
    }
    unsigned int childNum = static_cast<unsigned int>(node.children.size());
    modelBinaly.write(reinterpret_cast<const char*>(&childNum), sizeof(childNum));
    for (auto& child : node.children)
    {
        writeModelNode(modelBinaly, child);
    }
}

bool ModelImporter::Import(const std::wstring& filePath)
{
    ImportModelData modelData;
    if (!assimpImporter.Load(filePath, modelData))
    {
        return false;
    }

    std::string dir = MEngine::CacheDirectory + "model/";
    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directory(dir);
    }

    std::filesystem::path _filePath = filePath;
    std::ofstream modelBinaly(dir + _filePath.stem().string() + ".model", std::ios::binary);
    if (!modelBinaly.is_open())
    {
        return false;
    }

    writeModelNode(modelBinaly, modelData.model);
    modelBinaly.close();

    return true;
}
