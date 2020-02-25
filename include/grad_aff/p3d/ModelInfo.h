#pragma once

#include <vector>
#include <optional>

#include "../Types.h"

#include "Skeleton.h"

struct ModelInfo {
    uint32_t nLods = 0;
    std::vector<float_t> lodTypes = {};

    uint32_t index = 0;
    
    float_t memLodSpehre = 0;
    float_t geoLodSpehre = 0;

    std::vector<uint32_t> pointFlags = {};

    XYZTriplet offset1;
    uint32_t mapIconColor = 0;
    uint32_t mapSelectedColor = 0;
    float_t viewDensity = 0;

    XYZTriplet bboxMinPosition = {};
    XYZTriplet bboxMaxPosition = {};

    std::optional<float_t> lodDensityCoef = {};
    std::optional<float_t> drawImportance = {};
    std::optional<XYZTriplet> bboxMinVisual = {};
    std::optional<XYZTriplet> bboxMaxVisual = {};

    XYZTriplet centreOfGravity = {};
    XYZTriplet geometryCenter = {};
    XYZTriplet centerOfMass = {};

    std::vector<XYZTriplet> modelMassVectors = {};

    bool autoCenter = false;
    bool lockAutoCenter = false;
    bool canOcclude = false;
    bool canBeOccluded = false;

    std::optional<bool> aiCovers = {};

    std::optional<float_t> htMin = {};
    std::optional<float_t> htMax = {};
    std::optional<float_t> afMax = {};
    std::optional<float_t> mfMax = {};

    std::optional<float_t> mFact = {};
    std::optional<float_t> tBody = {};

    std::optional<bool> forceNotAlphaModel = {};

    std::optional<uint32_t> sbSource = {};
    std::optional<bool> preferShadowVolume = {};

    std::optional<float_t> shadowOffset = {};

    bool animated = false;

    Skeleton skeleton;

    uint8_t mapType = 0;

    uint32_t nFloats = 0;
    std::vector<float_t> unknownFloats = {};

    float_t mass = 0;
    float_t invMass = 0;
    float_t armor = 0;
    float_t invArmor = 0;

    std::optional<float_t> explosionShielding = {};
    std::optional<uint8_t> geometrySimple = {};
    std::optional<uint8_t> geometryPhys = {};

    uint8_t memory = 0;
    uint8_t geometry = 0;

    uint8_t geometryFire = 0;
    uint8_t geometryView = 0;
    uint8_t geometryViewPilot = 0;
    uint8_t geometryViewGunner = 0;

    // some geo View?
    uint8_t signedByte = 0;

    uint8_t geometryViewCargo = 0;

    uint8_t landContact = 0;
    uint8_t roadway = 0;
    uint8_t paths = 0;
    uint8_t hitPoints = 0;

    uint32_t minShadow = 0;

    std::optional<bool> canBlend = {};

    std::string propertyClass = "";
    std::string propertyDamage = "";
    std::string propertyFrequent = "";

    std::optional<uint32_t> unknownInt = {};

    std::vector<int32_t> preferredShadowVolumeLod = {};
    std::vector<int32_t> preferredShadowBufferLod = {};
    std::vector<int32_t> preferredShadowBufferLodVis = {};
};