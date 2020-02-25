#pragma once

#include <iostream>
#include <sstream>
#include <filesystem>

#include "../grad_aff.h"
#include "../StreamUtil.h"

#include "AnimationClass.h"
#include "Bones2Anims.h"
#include "Anims2Bones.h"
#include "FaceData.h"
#include "ODOLv4xLod.h"
#include "ModelInfo.h"

namespace fs = std::filesystem;

namespace grad_aff {
    class GRAD_AFF_API Odol {
    private:
        std::string filetype;

        std::shared_ptr<std::istream> is;


        std::map<float_t, LodType> lodMap = {
            { 1E+15f, LodType::SPECIAL_LOD},
            { 2E+13f, LodType::BUOYANCY },
            { 3E+13f, LodType::PHYSXOLD } ,
            { 4E+13f, LodType::PHYSX } ,
            { 1E+15f, LodType::MEMORY } ,
            { 2E+15f, LodType::LANDCONTACT } ,
            { 3E+15f, LodType::ROADWAY } ,
            { 4E+15f, LodType::PATHS },
            { 5E+15f, LodType::HITPOINTS } ,
            { 6E+15f, LodType::VIEW_GEOMETRY } ,
            { 7E+15f, LodType::FIRE_GEOMETRY },
            { 8E+15f, LodType::VIEW_GEOMETRY_CARGO },
            { 1.3E+16f, LodType::VIEW_GEOMETRY_PILOT },
            { 1.5E+16f, LodType::VIEW_GEOMETRY_GUNNER },
            { 1.6E+16f, LodType::FIRE_GEOMETRY_GUNNER },
            { 1.7E+16f, LodType::SUBPARTS },
            { 1.8E+16f, LodType::SHADOWVOLUME_CARGO},
            { 1.9E+16f, LodType::SHADOWVOLUME_PILOT},
            { 2E+16f, LodType::SHADOWVOLUME_GUNNER },
            { 2.1E+16f, LodType::WRECK },
            { 1E+16f, LodType::VIEW_COMMANDER },
            { 1000.0f, LodType::VIEW_GUNNER },
            { 1100.0f, LodType::VIEW_PILOT },
            { 1200.0f, LodType::VIEW_CARGO },
            { 1E+13f, LodType::GEOMETRY },
            { 4E+13f, LodType::PHYSX },
            { 10000.0f, LodType::SHADOWVOLUME },
            { 11000.0f, LodType::SHADOWBUFFER },
            { 10000.0f, LodType::SHADOW_MIN},
            { 20000.0f, LodType::SHADOW_MAX}
        };

    public:
        std::string signature = "";
        bool useCompression = false;
        bool useLzo = false;

        std::vector<uint32_t> startAddressOfLods = {};
        std::vector<uint32_t> endAddressOfLods = {};

        ModelInfo modelInfo;
        std::vector<ODOLv4xLod> lods = {};

        uint32_t version = 0;
        //uint32_t numberOfLods = 0;

        Odol(std::string filename);
        Odol(std::vector<uint8_t> data);

        void readOdol(bool withLods = true);

        void peekLodTypes();
        ODOLv4xLod readLod(uint32_t index);

        void readModelInfo(bool peekLodType = false);
        void readSkeleton();
        void readAnimations();
        ODOLv4xLod readLod();
        UVSet readUVSet();

        XYZTriplet decodeXYZ(uint32_t CompressedXYZ);
        std::vector<uint8_t> readLZOWithRule(uint32_t expectedSize);

        LodType getLodType(float_t resolution);
    };
};