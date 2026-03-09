#include "ModelLoader.h"

#include "MEngine.h"
#include <filesystem>
#include <fstream>

void readModelNode(std::ifstream& modelBinaly, ImportModelNode& node)
{
    size_t nameSize;
    modelBinaly.read(reinterpret_cast<char*>(&nameSize), sizeof(nameSize));
    node.name.resize(nameSize);
    modelBinaly.read(&node.name[0], nameSize);
    unsigned int meshNum;
    modelBinaly.read(reinterpret_cast<char*>(&meshNum), sizeof(meshNum));
    node.meshs.resize(meshNum);
    for (auto& mesh : node.meshs)
    {
        unsigned int vertexNum;
        modelBinaly.read(reinterpret_cast<char*>(&vertexNum), sizeof(vertexNum));
        mesh.data.vertices.resize(vertexNum);
        modelBinaly.read(reinterpret_cast<char*>(mesh.data.vertices.data()), vertexNum * sizeof(mesh.data.vertices[0]));
        unsigned int indexNum;
        modelBinaly.read(reinterpret_cast<char*>(&indexNum), sizeof(indexNum));
        mesh.data.indices.resize(indexNum);
        modelBinaly.read(reinterpret_cast<char*>(mesh.data.indices.data()), indexNum * sizeof(mesh.data.indices[0]));
    }
    unsigned int childNum;
    modelBinaly.read(reinterpret_cast<char*>(&childNum), sizeof(childNum));
    node.children.resize(childNum);
    for (auto& child : node.children)
    {
        readModelNode(modelBinaly, child);
    }
}

bool ModelLoader::Load(const std::wstring& path, ImportModelData& modelData)
{
    std::string dir = MEngine::CacheDirectory + "model/";
    std::filesystem::path _filePath = path;
    std::ifstream modelBinaly(dir + _filePath.stem().string() + ".model", std::ios::binary);

    if (!modelBinaly.is_open())
    {
        return false;
    }

    readModelNode(modelBinaly, modelData.model);

    modelBinaly.close();

    return true;
}
