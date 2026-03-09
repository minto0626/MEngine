#pragma once
#include <string>

#include "ModelImporter.h"

class ModelLoader
{
public:
    bool Load(const std::wstring& path, ImportModelData& modelData);
};
