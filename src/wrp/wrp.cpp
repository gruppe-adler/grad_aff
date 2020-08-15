#include "grad_aff/wrp/wrp.h"

grad_aff::Wrp::Wrp(std::string wrpFilename) {
    this->is = std::make_shared<std::ifstream>(wrpFilename, std::ios::binary);
    this->wrpName = wrpFilename;
};

grad_aff::Wrp::Wrp(std::vector<uint8_t> data) {
    this->is = std::make_shared<std::stringstream>(std::string(data.begin(), data.end()));
}

std::shared_ptr<grad_aff::GridBlockTree> grad_aff::Wrp::readABPair(std::istream& ifs, size_t dataSize) {
    std::vector<uint8_t> data;
    data.reserve(dataSize);
    for (size_t i = 0; i < dataSize; i++) {
        data.push_back(readBytes<uint8_t>(ifs));
    }
    return std::make_shared<GridBlockTree>(true, data);
}

std::shared_ptr<grad_aff::GridBlockTree> grad_aff::Wrp::readABPacket(std::istream& ifs, size_t dataSize) {
    auto subTree = std::make_shared<GridBlockTree>();
    auto flagBits = readBytes<uint16_t>(ifs);
    std::bitset<16> flagBitsSet(flagBits);
    for (int i = 0; i < flagBitsSet.size(); i++) {
        if (flagBitsSet[i]) {
            subTree->addChild(readABPacket(ifs, dataSize));
        }
        else {
            subTree->addChild(readABPair(ifs, dataSize));
        }
    }
    return subTree;
}

std::shared_ptr<grad_aff::GridBlockTree> grad_aff::Wrp::readGridBlock(std::istream& ifs, size_t dataSize) {
    auto isPresent = readBytes<uint8_t>(ifs);
    if (!isPresent) {
        auto nullBits = readBytes<uint32_t>(ifs);
        assert(nullBits == 0);
        return {};
    }
    else {
        return readABPacket(ifs, dataSize);
    }
};

void grad_aff::Wrp::readWrp()
{
    
    // TODO Checks
    this->wrpTypeName = readString(*is, 4);
    if(this->wrpTypeName != "OPRW") {
        throw std::runtime_error("Invalid file!");
    }
    this->wrpVersion = readBytes<uint32_t>(*is);
    // because why not right?
    // check which version
    if (wrpVersion > 24) {
        this->appId = readBytes<uint32_t>(*is);
    }
    this->layerSizeX = readBytes<uint32_t>(*is);
    this->layerSizeY = readBytes<uint32_t>(*is);
    this->mapSizeX = readBytes<uint32_t>(*is);
    this->mapSizeY = readBytes<uint32_t>(*is);

    this->mapSize = mapSizeX * mapSizeY;
    this->layerSize = layerSizeX * layerSizeY;

    this->layerCellSize = readBytes<float_t>(*is);
    this->geography = readGridBlock(*is, 4);

    this->cfgEnvSounds = readGridBlock(*is, 4);
    auto nPeaks = readBytes<uint32_t>(*is);

    peaks.resize(nPeaks);

    for (size_t i = 0; i < nPeaks; i++) {
        peaks[i] = readXYZTriplet(*is);
    }

    this->rvmatLayerIndex = readGridBlock(*is, 4);
    
    this->randomClutter = readLZOCompressed(*is, mapSize).first;
    this->compressedBytes = readLZOCompressed(*is, mapSize).first;
    this->elevation = readLZOCompressed<float_t>(*is, (size_t)mapSize * 4).first;

    auto nRvmats = readBytes<uint32_t>(*is);
    this->rvmats = std::vector<std::string>(nRvmats);

    for (size_t i = 0; i < nRvmats; i++) {
        rvmats[i] = readZeroTerminatedString(*is);
        bool b = readBytes<uint8_t>(*is) == 0;
        assert(b);
    }

    auto nModels = readBytes<uint32_t>(*is);
    this->models = std::vector<std::string>(nModels);

    for (size_t i = 0; i < nModels; i++) {
        models[i] = readZeroTerminatedString(*is);
    }

    auto nClassedModels = readBytes<uint32_t>(*is);
    this->classedModels = std::vector<ClassedModel>(nClassedModels);

    for (size_t i = 0; i < nClassedModels; i++) {
        ClassedModel classedModel;
        classedModel.className = readZeroTerminatedString(*is);
        classedModel.modelPath = readZeroTerminatedString(*is);
        classedModel.position = readXYZTriplet(*is);
        classedModel.unkonwn = readBytes<uint32_t>(*is);
        classedModels[i] = classedModel;
    }

    this->unknownGridBlock3 = readGridBlock(*is, 4);

    this->sizeOfObjects = readBytes<uint32_t>(*is);

    this->unknownGridBlock4 = readGridBlock(*is, 4);

    this->sizeOfMapInfo = readBytes<uint32_t>(*is);

    this->compressedBytes2 = readLZOCompressed(*is, layerSize).first;
    this->compressedBytes3 = readLZOCompressed(*is, mapSize).first;

    this->maxObjectId = readBytes<uint32_t>(*is);
    this->sizeOfRoadNets = readBytes<uint32_t>(*is);

    this->roadNets = std::vector<RoadNet>();// layerSize);

    for (size_t i = 0; i < layerSize; i++) {
        RoadNet roadNet;
        roadNet.nRoadParts = readBytes<uint32_t>(*is);

        roadNet.roadParts = std::vector<RoadPart>(roadNet.nRoadParts);

        for (size_t j = 0; j < roadNet.nRoadParts; j++) {
            RoadPart roadPart;
            roadPart.nRoadPositions = readBytes<uint16_t>(*is);

            roadPart.roadPositions = std::vector<XYZTriplet>(roadPart.nRoadPositions);

            size_t pos = is->tellg();

            if (roadPart.nRoadPositions > 10000) {
                auto pos = is->tellg();
                std::cout << pos << std::endl;
            }

            for (size_t k = 0; k < roadPart.nRoadPositions; k++) {
                roadPart.roadPositions[k] = readXYZTriplet(*is);
            }
            auto additionalBytes = readBytes(*is, roadPart.nRoadPositions);
            readBytes(*is, 4);

            roadPart.p3dModel = readZeroTerminatedString(*is);

            roadPart.transformMatrix = std::array<XYZTriplet, 4>();

            for (size_t k = 0; k < roadPart.transformMatrix.size(); k++) {
                roadPart.transformMatrix[k] = readXYZTriplet(*is);
            }

            roadNet.roadParts[j] = roadPart;
        }

        //roadNets[i] = roadNet;
        // TODO: remove when write is needed
        if (roadNet.nRoadParts != 0)
            roadNets.push_back(roadNet);

    }
    roadNets.shrink_to_fit();

    this->objects = std::vector<Object>(sizeOfObjects / GRAD_AFF_SIZE_OF_WRPOBJECT);

    for (size_t i = 0; i < objects.size(); i++) {
        Object object;
        object.objectId = readBytes<uint32_t>(*is);
        // index 0 -> no model?
        object.modelIndex = readBytes<uint32_t>(*is); // - 1?
        object.transformMatrix = std::array<XYZTriplet, 4>();
        for (size_t j = 0; j < object.transformMatrix.size(); j++) {
            object.transformMatrix[j] = readXYZTriplet(*is);
        }
        object.static0x02 = readBytes<uint32_t>(*is);
        assert(object.static0x02 == 0x02);

        objects[i] = object;
    }

    this->mapInfo = std::vector<std::shared_ptr<MapType>>();
    mapInfo.reserve(sizeOfMapInfo);

    while (!is->eof()) {
        auto infoType = readBytes<uint32_t>(*is);

        if (std::find(this->infoTypes1.begin(), this->infoTypes1.end(), infoType) != this->infoTypes1.end()) {
            auto mapData = std::make_shared<MapType1>();
            mapData->mapType = 1;
            mapData->infoType = infoType;
            mapData->objectId = readBytes<uint32_t>(*is);
            mapData->x = readBytes<float_t>(*is);
            mapData->y = readBytes<float_t>(*is);

            mapInfo.push_back(mapData);
        }
        else if (std::find(this->infoType2.begin(), this->infoType2.end(), infoType) != this->infoType2.end()) {
            auto mapData = std::make_shared<MapType2>();
            mapData->mapType = 2;
            mapData->infoType = infoType;
            mapData->objectId = readBytes<uint32_t>(*is);
            for (size_t i = 0; i < 8; i++) {
                mapData->bounds[i] = readBytes<float_t>(*is);
            }

            mapInfo.push_back(mapData);
        }
        else if (std::find(this->infoTypes3.begin(), this->infoTypes3.end(), infoType) != this->infoTypes3.end()) {
            auto mapData = std::make_shared<MapType3>();
            mapData->mapType = 3;
            mapData->infoType = infoType;
            mapData->color = readBytes<uint32_t>(*is);
            mapData->indicator = readBytes<uint32_t>(*is);
            for (size_t i = 0; i < 4; i++) {
                mapData->floats[i] = readBytes<float_t>(*is);
            }

            mapInfo.push_back(mapData);
        }
        else if (std::find(this->infoTypes4.begin(), this->infoTypes4.end(), infoType) != this->infoTypes4.end()) {
            auto mapData = std::make_shared<MapType4>();
            mapData->mapType = 4;
            mapData->infoType = infoType;
            mapData->objectId = readBytes<uint32_t>(*is);
            for (size_t i = 0; i < 8; i++) {
                mapData->bounds[i] = readBytes<float_t>(*is);
            }
            for (size_t i = 0; i < 4; i++) {
                mapData->color[i] = readBytes<uint8_t>(*is);
            }

            mapInfo.push_back(mapData);
        }
        else if (infoType == 34) {
            auto mapData = std::make_shared<MapType5>();
            mapData->mapType = 5;
            mapData->infoType = infoType;
            mapData->objectId = readBytes<uint32_t>(*is);
            for (size_t i = 0; i < 4; i++) {
                mapData->floats[i] = readBytes<float_t>(*is);
            }

            mapInfo.push_back(mapData);
        }
        else if (infoType == 35) {
            auto mapData = std::make_shared<MapType35>();
            mapData->mapType = 35;
            mapData->infoType = infoType;
            mapData->objectId = readBytes<uint32_t>(*is);
            for (size_t i = 0; i < 6; i++) {
                mapData->floats[i] = readBytes<float_t>(*is);
            }
            mapData->unknown = readBytes<uint8_t>(*is);

            mapInfo.push_back(mapData);
        }
        else {
            std::stringstream errorString;
            errorString << "Unkown infoType " << infoType << " encounterd at " << is->tellg() << " in " << this->wrpName << ". Please report this error at: https://github.com/gruppe-adler/grad_aff/issues";
            throw std::runtime_error(errorString.str());
        }
    }
    mapInfo.shrink_to_fit();
    
}
