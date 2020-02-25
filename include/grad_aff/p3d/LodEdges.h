#pragma once

#include <vector>

struct LodEdge {
    uint32_t nEdges = 0;
    std::vector<uint16_t> edges = {};
};

struct LodEdges {
    LodEdge lodEdge1; // MlodIndex
    LodEdge lodEdge2; // VertexIndex
};