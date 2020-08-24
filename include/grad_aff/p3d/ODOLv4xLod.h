#pragma once

#include <vector>
#include <map>

#include "LodProxy.h"
#include "LodBoneLink.h"
#include "LodMaterial.h"
#include "LodEdges.h"
#include "LodFaces.h"
#include "LodSection.h"
#include "LodNamedSelection.h"
#include "LodFrames.h"
#include "ClipFlag.h"
#include "UVSet.h"
#include "VertPorperty.h"
#include "VertexNeighborInfo.h"
#include "LodType.h"

struct ODOLv4xLod {
    LodType lodType;

    uint32_t nProxies = 0;
    std::vector<LodProxy> lodProxies = {};
    uint32_t nLodItems = 0;
    std::vector<uint32_t>  lodItems = {};
    uint32_t nBonesLinks = 0;
    std::vector<LodBoneLink> lodBoneLinks = {};

    std::optional<uint32_t> vertexCount = {};

    //std::optional<std::vector<LodPointFlag>> lodPointFlags = {};

    std::optional<float_t> faceArea = {};

    std::vector<ClipFlag> lodPointFlags = {};

    ClipFlag orHints = (ClipFlag)0;
    ClipFlag andHints = (ClipFlag)0;
    XYZTriplet bMin = {};
    XYZTriplet bMax = {};
    XYZTriplet bCeneter = {};
    float_t bRadius = 0;
    uint32_t nTextures = 0;
    std::vector<std::string> textures = {};
    uint32_t nMaterials = 0;
    std::vector<LodMaterial> lodMaterials = {};

    LodEdges lodEdges;

    uint32_t nFaces = 0;
    uint32_t offsetToSectionsStruct = 0;

    uint16_t alwaysZero = 0;

    std::vector<LodFace> lodFaces = {};

    uint32_t nSections = 0;
    std::vector<LodSection> lodSections = {};

    uint32_t nNamedSelections = 0;
    std::vector<LodNamedSelection> namedSelections = {};

    uint32_t nTokens = 0;
    std::map<std::string, std::string> tokens = {};

    uint32_t nFrames = 0;
    std::vector<LodFrame> lodFrames = {};

    uint32_t iconColor = 0;
    uint32_t selectedColor = 0;
    uint32_t special = 0;

    uint8_t vertexBoneRefIsImple = 0;
    uint32_t sizeOfVertexTable = 0;

    uint32_t nClipFlags = 0;
    std::vector<ClipFlag> clipFlags = {};

    // Vertex Table
    UVSet defaultUvSet;
    uint32_t nUvs = 0;
    std::vector<UVSet> uvSets = {};
    uint32_t nPoints = 0;
    std::vector<XYZTriplet> lodPoints = {};
    uint32_t nNormals = 0;
    std::vector<XYZTriplet> lodNormals = {};
    uint32_t nMinMax = 0;
    std::vector<XYZTriplet> lodMinMax = {};
    uint32_t nProperties = 0;
    std::vector<VertProperty> vertProperties = {};
    uint32_t count = 0;
    std::vector<VertexNeighborInfo> neighbourRef = {};



};