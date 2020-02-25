#pragma once

#include <vector>
/*
struct PsuedoVertexTable
{
    uint32_t PointsIndex = 0;
    uint32_t NormalsIndex = 0;
    float_t U = 0, V = 0;
};


struct LodFace {
    std::string textureName = "";
    uint32_t faceType = 0; // 3 = Triangle, 4 = Box
    std::array<PsuedoVertexTable, 4> psuedoVertexTable = {};
    uint32_t faceFlags = 0;
    std::string textureName = "";
    std::string materialName = "";
};
*/

struct LodFace {
    uint32_t faceFlags = 0;
    uint16_t textureIndex = 0;
    uint8_t faceType = 0;
    std::vector<uint16_t> vertexTableIndex = {};
};

struct LodFaces {
    uint32_t nFaces = 0;
    uint32_t allocationSizes = 0;
    std::vector<LodFace> lodFaces = {};
};