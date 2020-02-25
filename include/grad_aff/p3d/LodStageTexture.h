#pragma once 

#include <string>
#include <optional>

enum class TextureFilterType
{
    Point,
    Linear,
    Triliniear,
    Anisotropic
};

struct LodStageTexture {
    TextureFilterType textureFilter = TextureFilterType::Anisotropic;
    std::string paaTexture = "";
    uint32_t transFormIndex = 0;
    std::optional<bool> useWorldEnvMap = {};
};