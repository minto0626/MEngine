#include "AssimpImporter.h"
#include "ModelImporter.h"
#include "Utility/StringUtility.h"
#include "Math/Color.h"
#include "Utility/Debug.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <windows.h>
#include <filesystem>

void AssimpModelImporter::SetDirectoryAndLoadDll()
{
	char* rootPath = nullptr;
	size_t len = 0;
	if (_dupenv_s(&rootPath, &len, "ASSIMP_ROOT_DIR") == 0 && rootPath != nullptr)
	{
		std::string rootPathStr(rootPath);
		std::string dllName;
#if _DEBUG
		rootPathStr += "/build/bin/Debug";
		dllName = "assimp-vc143-mtd.dll";
#else
		rootPathStr += "/build/bin/Release";
		dllName = "assimp-vc143-mt.dll";
#endif
		SetDllDirectoryA(rootPathStr.c_str());
		LoadLibraryExA(dllName.c_str(), NULL, NULL);

        free(rootPath);
        rootPath = nullptr;
	}
}

void AssimpModelImporter::ParseNode(ImportModelNode& modelNode, aiNode* node, const aiScene* scene)
{
    modelNode.name = node->mName.C_Str();

    auto& meshList = modelNode.meshs;
    meshList.resize(node->mNumMeshes);

    for (auto i = 0; i < meshList.size(); ++i)
    {
        const auto mesh = scene->mMeshes[node->mMeshes[i]];
        ParseMesh(meshList[i], mesh);
    }

    auto& modelChilden = modelNode.children;
    modelChilden.resize(node->mNumChildren);
    for (auto i = 0; i < modelChilden.size(); ++i)
    {
        ParseNode(modelChilden[i], node->mChildren[i], scene);
    }
}

void AssimpModelImporter::ParseMesh(ImportMeshData& dstMesh, const aiMesh* srcMesh)
{
    dstMesh.name = srcMesh->mName.C_Str();

	aiVector3D zero3D(0.0f, 0.0f, 0.0f);

	// 頂点データを取得
	dstMesh.data.vertices.resize(srcMesh->mNumVertices);
	for (auto i = 0u; i < srcMesh->mNumVertices; ++i)
	{
		auto pos = &(srcMesh->mVertices[i]);
		auto normal = &(srcMesh->mNormals[i]);
		auto uv = (srcMesh->HasTextureCoords(0)) ? &(srcMesh->mTextureCoords[0][i]) : &zero3D;
		auto tangent = (srcMesh->HasTangentsAndBitangents()) ? &(srcMesh->mTangents[i]) : &zero3D;

		dstMesh.data.vertices[i] = Graphics::MeshVertex
		(
			Vector3(pos->x, pos->y, pos->z),
			Vector3(normal->x, normal->y, normal->z),
			Vector2(uv->x, uv->y)
		);
	}

	// 頂点インデックスを取得
	dstMesh.data.indices.resize(srcMesh->mNumFaces * 3);
	for (auto i = 0u; i < srcMesh->mNumFaces; ++i)
	{
		const auto& face = srcMesh->mFaces[i];
		assert(face.mNumIndices == 3);

		for (auto j = 0u; j < face.mNumIndices; ++j)
		{
			dstMesh.data.indices[i * 3 + j] = face.mIndices[j];
		}
	}

	// 参照するマテリアルのインデックスを取得
	dstMesh.materialIndex = srcMesh->mMaterialIndex;
}

void AssimpModelImporter::ParseMaterial(ImportMaterialData& dstMaterial, const aiMaterial* srcMaterial, std::string& rootDirectory)
{
    dstMaterial.name = srcMaterial->GetName().C_Str();

	// 拡散反射成分
	{
		aiColor3D color(0.5f, 0.5f, 0.5f);
		if (srcMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
		{
			dstMaterial.useDifuseColor = true;
			dstMaterial.diffuseColor = Color(color.r, color.g, color.b, 1.0f);
		}
		else
		{
			dstMaterial.useDifuseColor = false;
			dstMaterial.diffuseColor = Color(color.r, color.g, color.b, 1.0f);
		}
	}

	// 鏡面反射成分
	{
		aiColor3D color(0.0f, 0.0f, 0.0f);
		if (srcMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS)
		{
			dstMaterial.useSpecularColor = true;
			dstMaterial.specularColor = Color(color.r, color.g, color.b, 1.0f);
		}
		else
		{
			dstMaterial.useSpecularColor = false;
			dstMaterial.specularColor = Color(color.r, color.g, color.b, 1.0f);
		}
	}

	// 鏡面反射強度
	{
		float shininess = 0.0f;
		if (srcMaterial->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
		{
			dstMaterial.shininess = shininess;
		}
		else
		{
			dstMaterial.shininess = shininess;
		}
	}

	//ディフューズマップ
	{
		aiString path;
		if (srcMaterial->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path) == AI_SUCCESS)
		{
			dstMaterial.useDiffuseTexture = true;
			// テクスチャのパスを絶対パスに変換
			dstMaterial.diffuseTexturePath = rootDirectory + std::string(path.C_Str());
		}
		else
		{
			dstMaterial.useDiffuseTexture = false;
			dstMaterial.diffuseTexturePath.clear();
		}
	}
}

void AssimpModelImporter::Init()
{
	SetDirectoryAndLoadDll();
}

bool AssimpModelImporter::Load(const std::wstring& fileName, ImportModelData& modelData)
{
	auto filePath = StringUtility::ToUTF8String(fileName);

	Assimp::Importer importer;

	int flag = 0;
	flag |= aiProcess_ConvertToLeftHanded;
	flag |= aiProcess_Triangulate;
	flag |= aiProcess_PreTransformVertices;
	flag |= aiProcess_CalcTangentSpace;
	flag |= aiProcess_GenSmoothNormals;
	flag |= aiProcess_GenUVCoords;
	flag |= aiProcess_RemoveRedundantMaterials;
	flag |= aiProcess_OptimizeMeshes;

	auto scene = importer.ReadFile(filePath, flag);
	if (scene == nullptr || scene->mRootNode == nullptr)
	{
		Debug::LogError(("ファイルの読み込みに失敗しました。 path = " + filePath));
		return false;
	}

	auto rootDirectory = std::filesystem::path(filePath).parent_path().string();
	rootDirectory += "/";

    ParseNode(modelData.model, scene->mRootNode, scene);

    modelData.materials.resize(scene->mNumMaterials);
    for (auto i = 0; i < modelData.materials.size(); ++i)
    {
        const auto material = scene->mMaterials[i];
        ParseMaterial(modelData.materials[i], material, rootDirectory);
    }

#if _DEBUG
    std::stringstream log;
    log << "モデルのパス = " << filePath << std::endl;

    auto meshLog = [&](ImportModelNode& node)
        {
            log << "ノード名前: " << node.name << std::endl;
            for (auto i = 0; i < node.meshs.size(); ++i)
            {
                auto& mesh = node.meshs[i].data;
                log << "メッシュ[" << node.meshs[i].name << "] 頂点数 = " << mesh.vertices.size() << ", ";
                log << "インデックス数 = " << mesh.indices.size() << std::endl;
            }
        };
    auto materilLog = [&](ImportModelData& modelData)
        {
            for (auto i = 0; i < modelData.materials.size(); ++i)
            {
                log << "マテリアル[" << modelData.materials[i].name << "] を読み込みました。" << std::endl;
            }
        };
    meshLog(modelData.model);
    materilLog(modelData);

	log << "モデルの読み込みが完了";
	Debug::Log(log.str());
#endif

	return true;
}
