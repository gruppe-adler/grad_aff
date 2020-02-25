#pragma once

#include <string>
#include <vector>

struct LodNamedSelection {
    std::string selectedName = "";
    uint32_t nFaces;
    std::vector<uint16_t> faceIndexes = {};
    uint32_t alwaysZero;
    //std::vector

    bool isSectional = false;
    uint32_t nSections = 0;
    std::vector<uint32_t> sectionIndex = {};
    uint32_t nVertices = 0;
    std::vector<uint16_t> vertexTableIndexes = {};
    uint32_t nTextureWeights = 0;
    std::vector<uint8_t> verticesWeights = {};
};