#pragma once

#include "../grad_aff.h"
#include "../StreamUtil.h"
#include "../GridBlockTree.h"

#include "../a3lzo.h"

#include "ClassedModel.h"
#include "RoadNet.h"
#include "Object.h"
#include "MapInfo.h"

#include <iostream>
#include <vector>
#include <filesystem>
#include <map>
#include <cassert>
#include <bitset>
#include <sstream>
#include <algorithm>
#include <unordered_map>

namespace fs = std::filesystem;

namespace grad_aff {
    class GRAD_AFF_API Wrp {
    private:
        std::shared_ptr<std::istream> is;
        // infoTypes
        std::shared_ptr<GridBlockTree> readGridBlock(std::istream& ifs, size_t dataSize);
        std::shared_ptr<GridBlockTree> readABPacket(std::istream& ifs, size_t dataSize);
        std::shared_ptr<GridBlockTree> readABPair(std::istream& ifs, size_t dataSize);
    public:        
        const std::array<uint8_t, 16> infoTypes1 = { 0, 1, 2, 10, 11, 12, 13, 14, 15, 16, 17, 22, 23, 26, 27, 30 }; // 12 (cham)
        const std::array<uint8_t, 3> infoType2 = { 24, 31, 32 };
        const std::array<uint8_t, 5> infoTypes3 = { 25, 33, 41, 42, 43 }; // 41, 42, 43 (stratis)
        const std::array<uint8_t, 14> infoTypes4 = { 3, 4, 8, 9, 18, 19, 20, 21, 28, 29, 36, 37, 38, 39 }; // 36 (malden), 37,38 (altis), 39 (stratis) no doc

        Wrp(std::string filename);
        Wrp(std::vector<uint8_t> data);
        void readWrp();
        void writeWrp(fs::path path = "");

        std::string wrpName = "";
        std::string wrpTypeName = "";
        uint32_t wrpVersion = 0;
        uint32_t appId = 0;
        uint32_t layerSizeX = 0;
        uint32_t layerSizeY = 0;
        uint32_t mapSizeX = 0;
        uint32_t mapSizeY = 0;
        uint32_t mapSize = 0;
        uint32_t layerSize = 0;
        float_t layerCellSize = 0;

        std::shared_ptr<GridBlockTree> geography = {};
        std::shared_ptr<GridBlockTree> cfgEnvSounds = {};
        std::vector<XYZTriplet> peaks = {};
        std::shared_ptr<GridBlockTree> rvmatLayerIndex = {};

        std::vector<uint8_t> randomClutter = {};
        std::vector<uint8_t> compressedBytes = {};
        std::vector<float_t> elevation = {};
        std::vector<std::string> rvmats = {};
        std::vector<std::string> models = {};
        std::vector<ClassedModel> classedModels = {};

        std::shared_ptr<GridBlockTree> unknownGridBlock3 = {};
        uint32_t sizeOfObjects = 0;
        std::shared_ptr<GridBlockTree> unknownGridBlock4 = {};
        uint32_t sizeOfMapInfo = 0;

        std::vector<uint8_t> compressedBytes2 = {};
        std::vector<uint8_t> compressedBytes3 = {};

        uint32_t maxObjectId = 0;
        uint32_t sizeOfRoadNets = 0;

        std::vector<RoadNet> roadNets = {};
        std::vector<Object> objects = {};
        std::vector<std::shared_ptr<MapType>> mapInfo = {};
        std::unordered_map<uint32_t, Object> objectIdMap = {};
    };
}