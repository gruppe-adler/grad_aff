#pragma once

#include <vector>
#include <string>
#include <optional>

#include "../Types.h"
#include "LodStageTexture.h"
#include "LodStageTransform.h"
#include "ShaderId.h"

struct LodMaterial {
    std::string rvMatName = "";
    uint32_t type = 0;

    D3DCOLORVALUE emissive = {};
    D3DCOLORVALUE ambient = {};
    D3DCOLORVALUE diffuse = {};
    D3DCOLORVALUE forcedDiffuse = {};
    D3DCOLORVALUE specular = {};
    D3DCOLORVALUE specular2 = {};

    float_t specularPower = 0;
    PixelShaderID pixelShader;
    VertexShaderID vertexShader;
    EMainLight mainLight;
    EFogMode fogMode;

    std::optional<bool> unkBool = {};

    std::optional<std::string> surfaceFile = {};

    std::optional<uint32_t> nRenderFlags = {};
    std::optional<uint32_t> renderFlags = {};
    
    std::optional<uint32_t> nStages = {};

    std::optional<uint32_t> nTexGens = {};

    std::vector<LodStageTexture> stageTexures = {};
    std::vector<LodStageTransform> stageTransforms = {};

    std::vector<LodStageTexture> dummyStageTexture = {};

};