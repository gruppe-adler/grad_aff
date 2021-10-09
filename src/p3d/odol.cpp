#include "grad_aff/p3d/odol.h"


grad_aff::Odol::Odol(std::string filename) {
    this->is = std::make_shared<std::ifstream>(filename, std::ios::binary);
};

grad_aff::Odol::Odol(std::vector<uint8_t> data) {
    this->is = std::make_shared<std::stringstream>(std::string(data.begin(), data.end()));
}

void grad_aff::Odol::readOdol(bool withLods) {
    is->seekg(0);
    signature = readString(*is, 4);
    assert(signature == "ODOL");
    version = readBytes<uint32_t>(*is);

    if (version < 40) {
        throw std::runtime_error("non arma odols are not supported!");
    }

    if (version > 73) {
        throw std::runtime_error("unknown odol version!");
    }

    if (version >= 44) {
        this->useLzo = true;
    }

    if (version >= 64) {
        this->useCompression = true;
    }

    if (version == 58) {
        auto p3dPrefix = readZeroTerminatedString(*is);
    }

    if (version >= 59) {
        auto appId = readBytes<uint32_t>(*is);
    }
    if(version >= 58) {
        auto muzzleFlashString = readZeroTerminatedString(*is);
    }

    readModelInfo();
    readAnimations();

    
    for (auto i = 0; i < modelInfo.nLods; i++) {
        startAddressOfLods.push_back(readBytes<uint32_t>(*is));
    }

    for (auto i = 0; i < modelInfo.nLods; i++) {
        endAddressOfLods.push_back(readBytes<uint32_t>(*is));
    }

    std::vector<bool> useDefault = {};
    for (auto i = 0; i < modelInfo.nLods; i++) {
        useDefault.push_back(readBytes<bool>(*is));
    }

    auto useDefaultFalseCount = 0;
    for (auto useDefaultVal : useDefault) {
        if (!useDefaultVal)
            useDefaultFalseCount++;
    }

    std::vector<FaceData> faceDefaults = {};
    for (auto useDefaultVal : useDefault) {
        FaceData faceData;

        if (useDefaultVal) {
            faceDefaults.push_back({});
            continue;
        }

        faceData.headerFaceCount = readBytes<int32_t>(*is);
        faceData.color = readBytes<uint32_t>(*is);
        faceData.special = readBytes<uint32_t>(*is);
        faceData.orHints = readBytes<uint32_t>(*is);

        if (version >= 39) {
            faceData.hasSkeleton = readBytes<bool>(*is);
        }

        if (version >= 51) {
            faceData.nVertices = readBytes<uint32_t>(*is);
            faceData.faceArea = readBytes<float_t>(*is);
        }

        faceDefaults.push_back(faceData);
    }

    if (withLods) {
        this->lods.clear();
        for (auto i = 0; i < modelInfo.nLods; i++) {
            is->seekg(startAddressOfLods[i]);
            this->lods.push_back(readLod());
            lods.back().lodType = getLodType(modelInfo.lodTypes[i]);
        }
    }
}

void grad_aff::Odol::peekLodTypes() {
    if (modelInfo.nLods == 0)
        readOdol(false);

    this->lods.clear();
    for (auto i = 0; i < modelInfo.nLods; i++) {
        is->seekg(startAddressOfLods[i]);
        this->lods.push_back(readLod());
        lods.back().lodType = getLodType(modelInfo.lodTypes[i]);
    }
}

ODOLv4xLod grad_aff::Odol::readLod(uint32_t index) {
    if (modelInfo.nLods == 0)
        readOdol(false);

    is->seekg(startAddressOfLods[index]);
    ODOLv4xLod lod = readLod();
    lod.lodType = getLodType(modelInfo.lodTypes[index]);
    return lod;
}

void grad_aff::Odol::readModelInfo(bool peekLodType) {
    modelInfo.nLods = readBytes<uint32_t>(*is);

    modelInfo.lodTypes.reserve(modelInfo.nLods);
    for (size_t i = 0; i < modelInfo.nLods; i++) {
        modelInfo.lodTypes.push_back(readBytes<float_t>(*is));
    }

    if (peekLodType)
        return;

    modelInfo.index = readBytes<uint32_t>(*is);

    modelInfo.memLodSpehre = readBytes<float_t>(*is);
    modelInfo.geoLodSpehre = readBytes<float_t>(*is);

    for (size_t i = 0; i < 3; i++) {
        modelInfo.pointFlags.push_back(readBytes<uint32_t>(*is));
    }

    modelInfo.offset1 = readXYZTriplet(*is);
    modelInfo.mapIconColor = readBytes<uint32_t>(*is);
    modelInfo.mapSelectedColor = readBytes<uint32_t>(*is);

    modelInfo.viewDensity = readBytes<float_t>(*is);

    modelInfo.bboxMinPosition = readXYZTriplet(*is);
    modelInfo.bboxMaxPosition = readXYZTriplet(*is);

    if (version >= 70) {
        modelInfo.lodDensityCoef = readBytes<float_t>(*is);
    }

    if (version >= 71) {
        modelInfo.drawImportance = readBytes<float_t>(*is);
    }

    if (version >= 52) {
        modelInfo.bboxMinVisual = readXYZTriplet(*is);
        modelInfo.bboxMaxVisual = readXYZTriplet(*is);
    }

    modelInfo.centreOfGravity = readXYZTriplet(*is);
    modelInfo.geometryCenter = readXYZTriplet(*is);
    modelInfo.centerOfMass = readXYZTriplet(*is);

    for (size_t i = 0; i < 3; i++) {
        modelInfo.modelMassVectors.push_back(readXYZTriplet(*is));
    }
    /*
    std::vector<uint8_t> thermalProfile;
    for (size_t i = 0; i < 24; i++) {
        thermalProfile.push_back(readBytes<uint8_t>(*is));
    }
    */

    modelInfo.autoCenter = readBytes<bool>(*is);
    modelInfo.lockAutoCenter = readBytes<bool>(*is);
    modelInfo.canOcclude = readBytes<bool>(*is);
    modelInfo.canBeOccluded = readBytes<bool>(*is);

    if (version >= 73) {
        modelInfo.aiCovers = readBytes<bool>(*is);
    }

    if (version >= 42) {
        modelInfo.htMin = readBytes<float_t>(*is);
        modelInfo.htMax = readBytes<float_t>(*is);
        modelInfo.afMax = readBytes<float_t>(*is);
        modelInfo.mfMax = readBytes<float_t>(*is);
    }

    if (version >= 43) {
        modelInfo.mFact = readBytes<float_t>(*is);
        modelInfo.tBody = readBytes<float_t>(*is);
    }

    if (version >= 33) {
        modelInfo.forceNotAlphaModel = readBytes<bool>(*is);
    }

    if (version >= 37) {
        modelInfo.sbSource = readBytes<uint32_t>(*is);
        modelInfo.preferShadowVolume = readBytes<bool>(*is);
    }

    if (version >= 48) {
        modelInfo.shadowOffset = readBytes<float_t>(*is);
    }

    modelInfo.animated = readBytes<bool>(*is);
    readSkeleton();

    modelInfo.mapType = readBytes<uint8_t>(*is);

    modelInfo.nFloats = readBytes<uint32_t>(*is);
    modelInfo.unknownFloats = readCompressedArray<float_t>(*is, 4, useCompression, modelInfo.nFloats); // mass array?

    modelInfo.mass = readBytes<float_t>(*is);
    modelInfo.invMass = readBytes<float_t>(*is);
    modelInfo.armor = readBytes<float_t>(*is);
    modelInfo.invArmor = readBytes<float_t>(*is);
    
    if (version >= 72) {
        modelInfo.explosionShielding = readBytes<float_t>(*is);
    }

    if (version >= 53) {
        modelInfo.geometrySimple = readBytes<uint8_t>(*is);
    }

    if (version >= 54) {
        modelInfo.geometryPhys = readBytes<uint8_t>(*is);
    }

    modelInfo.memory = readBytes<uint8_t>(*is);
    modelInfo.geometry = readBytes<uint8_t>(*is);

    modelInfo.geometryFire = readBytes<uint8_t>(*is);
    modelInfo.geometryView = readBytes<uint8_t>(*is);
    modelInfo.geometryViewPilot = readBytes<uint8_t>(*is);
    modelInfo.geometryViewGunner = readBytes<uint8_t>(*is);

    // some geo View?
    modelInfo.signedByte = readBytes<int8_t>(*is);

    modelInfo.geometryViewCargo = readBytes<uint8_t>(*is);

    modelInfo.landContact = readBytes<uint8_t>(*is);
    modelInfo.roadway = readBytes<uint8_t>(*is);
    modelInfo.paths = readBytes<uint8_t>(*is);
    modelInfo.hitPoints = readBytes<uint8_t>(*is);

    modelInfo.minShadow = readBytes<uint32_t>(*is);

    if (version >= 38) {
        modelInfo.canBlend = readBytes<bool>(*is);
    }

    modelInfo.propertyClass = readZeroTerminatedString(*is);
    modelInfo.propertyDamage = readZeroTerminatedString(*is);
    modelInfo.propertyFrequent = readBytes<bool>(*is);

    //bool allowAnimation = readBytes<bool>(*is);
    /*
    std::vector<uint8_t> unkArmaFlags;
    for (size_t i = 0; i < 24; i++) {
        unkArmaFlags.push_back(readBytes<uint8_t>(*is));
    }
    */
    if (version >= 31) {
        modelInfo.unknownInt = readBytes<uint32_t>(*is);
    }

    if (version >= 57) {
        for (auto i = 0; i < modelInfo.nLods; i++) {
            modelInfo.preferredShadowVolumeLod.push_back(readBytes<int32_t>(*is));
        }

        for (auto i = 0; i < modelInfo.nLods; i++) {
            modelInfo.preferredShadowBufferLod.push_back(readBytes<int32_t>(*is));
        }

        for (auto i = 0; i < modelInfo.nLods; i++) {
            modelInfo.preferredShadowBufferLodVis.push_back(readBytes<int32_t>(*is));
        }

    }

}

void grad_aff::Odol::readSkeleton() {
    modelInfo.skeleton.name = readZeroTerminatedString(*is);
    if (modelInfo.skeleton.name == "")
        return;

    if (version >= 23)
    {
        modelInfo.skeleton.isDiscrete = readBytes<bool>(*is);
    }

    modelInfo.skeleton.nBones = readBytes<uint32_t>(*is);
    for (auto i = 0; i < modelInfo.skeleton.nBones; i++) {
        modelInfo.skeleton.bones.push_back(readZeroTerminatedString(*is));
        modelInfo.skeleton.bones.push_back(readZeroTerminatedString(*is));
    }

    if (version > 40) {
        modelInfo.skeleton.pivotsNameObsolete = readZeroTerminatedString(*is);
    }
}

void grad_aff::Odol::readAnimations() {
    auto animsExist = readBytes<bool>(*is);
    if (!animsExist)
        return;

    auto nAnimationClasses = readBytes<uint32_t>(*is);
    std::vector<AnimationClass> animationClasses = {};
    for (auto i = 0; i < nAnimationClasses; i++) {
        AnimationClass animationClass;
        animationClass.animTransformType = readBytes<uint32_t>(*is);
        animationClass.animClassName = readZeroTerminatedString(*is);
        animationClass.animSource = readZeroTerminatedString(*is);

        animationClass.minValue = readBytes<float_t>(*is);
        animationClass.maxValue = readBytes<float_t>(*is);
        animationClass.minPhase = readBytes<float_t>(*is);
        animationClass.maxPhase = readBytes<float_t>(*is);

        animationClass.sourceAddress = readBytes<uint32_t>(*is);

        if (version >= 56) {
            animationClass.animPeriod = readBytes<uint32_t>(*is);
            animationClass.initPhase = readBytes<uint32_t>(*is);
        }

        switch ((AnimTransformTypeEnum)animationClass.animTransformType)
        {
        case AnimTransformTypeEnum::ROTATION:
        case AnimTransformTypeEnum::ROTATIONX:
        case AnimTransformTypeEnum::ROTATIONY:
        case AnimTransformTypeEnum::ROTATIONZ:
        {
            auto animTransformRotation = std::make_shared<AnimTransformRotation>();
            animTransformRotation->type = (AnimTransformTypeEnum)animationClass.animTransformType;
            animTransformRotation->angle0 = readBytes<float_t>(*is);
            animTransformRotation->angle1 = readBytes<float_t>(*is);
            animationClass.animType = animTransformRotation;
        }
            break;
        case AnimTransformTypeEnum::TRANSLATION:
        case AnimTransformTypeEnum::TRANSLATIONX:
        case AnimTransformTypeEnum::TRANSLATIONY:
        case AnimTransformTypeEnum::TRANSLATIONZ:
        {
            auto animTransformTranslation = std::make_shared<AnimTransformTranslation>();
            animTransformTranslation->type = (AnimTransformTypeEnum)animationClass.animTransformType;
            animTransformTranslation->offset0 = readBytes<float_t>(*is);
            animTransformTranslation->offset1 = readBytes<float_t>(*is);
            animationClass.animType = animTransformTranslation;
        }
            break;
        case AnimTransformTypeEnum::DIRECT:
        {
            auto animTransformDirect = std::make_shared<AnimTransformDirect>();
            animTransformDirect->type = AnimTransformTypeEnum::DIRECT;
            animTransformDirect->axisPos = readXYZTriplet(*is);
            animTransformDirect->axisDir = readXYZTriplet(*is);
            animTransformDirect->angle = readBytes<float_t>(*is);
            animTransformDirect->axisOffset = readBytes<float_t>(*is);
            animationClass.animType = animTransformDirect;
        }
            break;
        case AnimTransformTypeEnum::HIDE:
        {
            auto animTransformHide = std::make_shared<AnimTransformHide>();
            animTransformHide->type = AnimTransformTypeEnum::HIDE;
            animTransformHide->hideValue = readBytes<float_t>(*is);
            if (version >= 55) {
                animTransformHide->unknownFloat = readBytes<float_t>(*is);
            }
            animationClass.animType = animTransformHide;
        }
            break;
        default:
            throw std::runtime_error("Unknown AnimType: " + animationClass.animTransformType);
            break;
        }

        animationClasses.push_back(animationClass);
    }

    auto nResolutions = readBytes<uint32_t>(*is);

    std::vector<Bones2Anims> bones2AnimsList = {};
    for (auto i = 0; i < nResolutions; i++) {
        Bones2Anims bones2Anims;
        bones2Anims.nBones = readBytes<uint32_t>(*is);

        for (auto j = 0; j < bones2Anims.nBones; j++) {
            Bone2AnimClassList bone2AnimClassList;
            bone2AnimClassList.nAnimClasses = readBytes<uint32_t>(*is);

            for (auto k = 0; k < bone2AnimClassList.nAnimClasses; k++) {
                bone2AnimClassList.animationClassIndex.push_back(readBytes<uint32_t>(*is));
            }
            bones2Anims.bone2AnimClassLists.push_back(bone2AnimClassList);;
        }
        bones2AnimsList.push_back(bones2Anims);
    }

    std::vector<Anims2Bones> anims2Bones = {};
    for (auto i = 0; i < nResolutions; i++) {
        Anims2Bones anim2Bones;
        for (auto j = 0; j < nAnimationClasses; j++) {
            AnimBones animBones;
            animBones.skeletonBoneNameIndex = readBytes<int32_t>(*is);

            if (animBones.skeletonBoneNameIndex != -1 && 
                animationClasses[j].animType->type != AnimTransformTypeEnum::DIRECT && animationClasses[j].animType->type != AnimTransformTypeEnum::HIDE) {
                animBones.axisPos = readXYZTriplet(*is);
                animBones.axisDir = readXYZTriplet(*is);
            }
            anim2Bones.animBones.push_back(animBones);
        }
        anims2Bones.push_back(anim2Bones);
    }
}

ODOLv4xLod grad_aff::Odol::readLod() {
    ODOLv4xLod lod;
    lod.nProxies = readBytes<uint32_t>(*is);
    lod.lodProxies.reserve(lod.nProxies);
    
    for (auto i = 0; i < lod.nProxies; i++) {
        LodProxy lodProxy;
        lodProxy.p3dProxyName = readZeroTerminatedString(*is);
        lodProxy.transform = readMatrix(*is);
        lodProxy.proxySeqenceID = readBytes<int32_t>(*is);
        lodProxy.namedSelectionIndex = readBytes<int32_t>(*is);
        lodProxy.boneIndex = readBytes<int32_t>(*is);
        if (this->version >= 40) {
            lodProxy.sectionIndex = readBytes<int32_t>(*is);
        }
        lod.lodProxies.push_back(lodProxy);
    }

    lod.nLodItems = readBytes<uint32_t>(*is);
    lod.lodItems.reserve(lod.nLodItems);
    for (auto i = 0; i < lod.nLodItems; i++) {
        lod.lodItems.push_back(readBytes<uint32_t>(*is));
    }

    lod.nBonesLinks = readBytes<uint32_t>(*is);
    lod.lodBoneLinks.reserve(lod.nBonesLinks);
    for (auto i = 0; i < lod.nBonesLinks; i++) {
        LodBoneLink lodBoneLink;
        lodBoneLink.nLinks = readBytes<uint32_t>(*is);
        for (auto j = 0; j < lodBoneLink.nLinks; j++) {
            lodBoneLink.link.push_back(readBytes<uint32_t>(*is));
        }
        lod.lodBoneLinks.push_back(lodBoneLink);
    }

    if (version >= 50) {
        lod.vertexCount = readBytes<uint32_t>(*is);
    }
    else {
        auto compressedArray = readCompressedFillArray<uint32_t>(*is, useCompression);

        lod.lodPointFlags.clear();
        lod.lodPointFlags.reserve(compressedArray.size());
        std::transform(compressedArray.begin(), compressedArray.end(), std::back_inserter(lod.lodPointFlags), [](int n) { 
            return static_cast<ClipFlag>(n); 
        });
    }

    if (version >= 51) {
        lod.faceArea = readBytes<float_t>(*is);
    }

    lod.orHints = static_cast<ClipFlag>(readBytes<uint32_t>(*is));
    lod.andHints = static_cast<ClipFlag>(readBytes<uint32_t>(*is));

    lod.bMin = readXYZTriplet(*is);
    lod.bMax = readXYZTriplet(*is);
    lod.bCeneter = readXYZTriplet(*is);
    lod.bRadius = readBytes<float_t>(*is);

    lod.nTextures = readBytes<uint32_t>(*is);
    lod.textures.reserve(lod.nTextures);
    for (auto i = 0; i < lod.nTextures; i++) {
        lod.textures.push_back(readZeroTerminatedString(*is));
    }

    lod.nMaterials = readBytes<uint32_t>(*is);
    lod.lodMaterials.reserve(lod.nMaterials);
    for (auto i = 0; i < lod.nMaterials; i++) {
        LodMaterial lodMaterial;
        lodMaterial.rvMatName = readZeroTerminatedString(*is);
        lodMaterial.type = readBytes<uint32_t>(*is);

        lodMaterial.emissive = readD3ColorValue(*is);
        lodMaterial.ambient = readD3ColorValue(*is);
        lodMaterial.diffuse = readD3ColorValue(*is);
        lodMaterial.forcedDiffuse = readD3ColorValue(*is);
        lodMaterial.specular = readD3ColorValue(*is);
        lodMaterial.specular2 = readD3ColorValue(*is);

        lodMaterial.specularPower = readBytes<float_t>(*is);

        lodMaterial.pixelShader = (PixelShaderID)readBytes<uint32_t>(*is);
        lodMaterial.vertexShader = (VertexShaderID)readBytes<uint32_t>(*is);
        lodMaterial.mainLight = (EMainLight)readBytes<uint32_t>(*is);
        lodMaterial.fogMode = (EFogMode)readBytes<uint32_t>(*is);

        if (lodMaterial.type == 3) {
            lodMaterial.unkBool = readBytes<bool>(*is);
        }

        if (lodMaterial.type >= 6) {
            lodMaterial.surfaceFile = readZeroTerminatedString(*is);
        }

        if (lodMaterial.type >= 4) {
            lodMaterial.nRenderFlags = readBytes<uint32_t>(*is);
            lodMaterial.renderFlags = readBytes<uint32_t>(*is);
        }

        if (lodMaterial.type > 6) {
            lodMaterial.nStages = readBytes<uint32_t>(*is);
        }
        if (lodMaterial.type > 8) {
            lodMaterial.nTexGens = readBytes<uint32_t>(*is);
        }
        auto posD3 = (size_t)is->tellg();
        if (lodMaterial.type < 8) {
            throw std::runtime_error("TODO implement");
        }
        else {
            for (auto j = 0; j < lodMaterial.nStages; j++) {
                LodStageTexture stageTexture;
                if (lodMaterial.type >= 5) {
                    stageTexture.textureFilter = (TextureFilterType)readBytes<uint32_t>(*is);
                }
                stageTexture.paaTexture = readZeroTerminatedString(*is);
                if (lodMaterial.type >= 8) {
                    stageTexture.transFormIndex = readBytes<uint32_t>(*is);
                }
                if (lodMaterial.type >= 11) {
                    stageTexture.useWorldEnvMap = readBytes<bool>(*is);
                }
                lodMaterial.stageTexures.push_back(stageTexture);
            }

            for (auto j = 0; j < lodMaterial.nTexGens; j++) {
                LodStageTransform stageTransform;
                stageTransform.uvSource = (UVSource)readBytes<uint32_t>(*is);
                stageTransform.transFormMatrix = readMatrix(*is);
                lodMaterial.stageTransforms.push_back(stageTransform);
            }
        }
        if (lodMaterial.type >= 10) {
            LodStageTexture dummyStageTexture;
            if (lodMaterial.type >= 5) {
                dummyStageTexture.textureFilter = (TextureFilterType)readBytes<uint32_t>(*is);
            }
            dummyStageTexture.paaTexture = readZeroTerminatedString(*is);
            if (lodMaterial.type >= 8) {
                dummyStageTexture.transFormIndex = readBytes<uint32_t>(*is);
            }
            if (lodMaterial.type >= 11) {
                dummyStageTexture.useWorldEnvMap = readBytes<bool>(*is);
            }
            lodMaterial.dummyStageTexture.push_back(dummyStageTexture);
        }
        lod.lodMaterials.push_back(lodMaterial);
    }

    auto pointToVertex = readCompressedArray<uint32_t>(*is, (version >= 69 ? 4 : 2), false);
    auto vertexToPoint = readCompressedArray<uint32_t>(*is, (version >= 69 ? 4 : 2), false);

    /*
    LodEdges lodEdges;

    LodEdge lodEdge1;
    lodEdge1.nEdges = readBytes<uint32_t>(*is);
    for (auto i = 0; i < lodEdge1.nEdges; i++) {
        lodEdge1.edges.push_back(readBytes<uint16_t>(*is));
    }
    lodEdges.lodEdge1 = lodEdge1;

    LodEdge lodEdge2;
    lodEdge2.nEdges = readBytes<uint32_t>(*is);
    for (auto i = 0; i < lodEdge2.nEdges; i++) {
        lodEdge2.edges.push_back(readBytes<uint16_t>(*is));
    }
    lodEdges.lodEdge2 = lodEdge2;

    lod.lodEdges = lodEdges;
    */
    
    lod.nFaces = readBytes<uint32_t>(*is);
    lod.offsetToSectionsStruct = readBytes<uint32_t>(*is);
    lod.alwaysZero = readBytes<uint16_t>(*is);

    for (auto i = 0; i < lod.nFaces; i++) {
        LodFace lodFace;
        lodFace.faceType = readBytes<uint8_t>(*is);
        for (auto j = 0; j < lodFace.faceType; j++) {
            if (version >= 69) {
                lodFace.vertexTableIndex.push_back(readBytes<uint32_t>(*is));
            }
            else {
                lodFace.vertexTableIndex.push_back(readBytes<uint16_t>(*is));
            }

        }
        lod.lodFaces.push_back(lodFace);
    }

    lod.nSections = readBytes<uint32_t>(*is);
    for (auto i = 0; i < lod.nSections; i++) {
        LodSection lodSection;
        lodSection.faceLowerIndex = readBytes<int32_t>(*is);
        lodSection.faceUpperIndex = readBytes<int32_t>(*is);
        lodSection.minBonexIndex = readBytes<int32_t>(*is);
        lodSection.bonesCount = readBytes<int32_t>(*is);
        lodSection.commonPointsUserValue = readBytes<uint32_t>(*is);
        lodSection.commonTextureIndex = readBytes<int16_t>(*is);
        lodSection.commonFaceFlags = readBytes<uint32_t>(*is);
        lodSection.materialIndex = readBytes<int32_t>(*is);

        if (lodSection.materialIndex == -1) {
            lodSection.material = readZeroTerminatedString(*is);
        }

        if (version >= 36) {
            lodSection.nStages = readBytes<uint32_t>(*is);
            for(auto j = 0; j < lodSection.nStages; j++) {
                lodSection.areaOverTex.push_back(readBytes<float_t>(*is));
            }
            
            if (version >= 67 && readBytes<uint32_t>(*is) >= 1) {
                for (auto j = 0; j < 12; j++) {
                    (lodSection.floats)[j] = readBytes<float_t>(*is);
                }
            }

        }
        else {
            lodSection.nStages = 1;
            lodSection.areaOverTex.push_back(readBytes<float_t>(*is));
        }
        lod.lodSections.push_back(lodSection);
    }

    lod.nNamedSelections = readBytes<uint32_t>(*is);
    for (auto i = 0; i < lod.nNamedSelections; i++) {
        LodNamedSelection lodNamedSelection;
        lodNamedSelection.selectedName = readZeroTerminatedString(*is);

        //lodNamedSelection.nFaces = readBytes<uint32_t>(*is);
        lodNamedSelection.faceIndexes = readCompressedArray<uint16_t>(*is, (version >= 69 ? 4 : 2), useCompression);
        /*
        if (version >= 69) {
            lodNamedSelection.faceIndexes = readLZOCompressed<uint16_t>(*is, (size_t)lodNamedSelection.nFaces * 4).first;
        }
        else {
            lodNamedSelection.faceIndexes = readLZOCompressed<uint16_t>(*is, (size_t)lodNamedSelection.nFaces * 2).first;
        }
        */
        lodNamedSelection.alwaysZero = readBytes<uint32_t>(*is);
        lodNamedSelection.isSectional = readBytes<bool>(*is);
        lodNamedSelection.sectionIndex = readCompressedArray<uint32_t>(*is, 4, useCompression);
        lodNamedSelection.nSections = lodNamedSelection.sectionIndex.size();
        lodNamedSelection.vertexTableIndexes = readCompressedArray<uint16_t>(*is, (version >= 69 ? 4 : 2), useCompression);
        /*lodNamedSelection.nVertices = readBytes<uint32_t>(*is);
        if (version >= 69) {
            lodNamedSelection.vertexTableIndexes = readLZOCompressed<uint16_t>(*is, (size_t)lodNamedSelection.nVertices * 4).first;
        }
        else {
            lodNamedSelection.vertexTableIndexes = readLZOCompressed<uint16_t>(*is, (size_t)lodNamedSelection.nVertices * 2).first;
        }
        */
        lodNamedSelection.nTextureWeights = readBytes<uint32_t>(*is);
        //lodNamedSelection.verticesWeights = readLZOCompressed<uint8_t>(*is, lodNamedSelection.nTextureWeights).first;
        lodNamedSelection.verticesWeights = readCompressed(*is, lodNamedSelection.nTextureWeights, this->useCompression);

        lod.namedSelections.push_back(lodNamedSelection);
    }

    lod.nTokens = readBytes<uint32_t>(*is);
    for (auto i = 0; i < lod.nTokens; i++) {
        lod.tokens.insert({ readZeroTerminatedString(*is), readZeroTerminatedString(*is) });
    }

    lod.nFrames = readBytes<uint32_t>(*is);
    for (auto i = 0; i < lod.nFrames; i++) {
        LodFrame lodFrame;
        lodFrame.frameTime = readBytes<float_t>(*is);
        lodFrame.nBones = readBytes<uint32_t>(*is);
        for (auto k = 0; k < lodFrame.nBones; k++) {
            lodFrame.bonePositions.push_back(readXYZTriplet(*is));
        }
        lod.lodFrames.push_back(lodFrame);
    }

    lod.iconColor = readBytes<uint32_t>(*is);
    lod.selectedColor = readBytes<uint32_t>(*is);
    lod.special = readBytes<uint32_t>(*is);
    lod.vertexBoneRefIsImple = readBytes<bool>(*is);
    lod.sizeOfVertexTable = readBytes<uint32_t>(*is);

    if (version >= 50) {
        lod.nClipFlags = readBytes<uint32_t>(*is);
        if (readBytes<bool>(*is)) {
            auto val = (ClipFlag)readBytes<uint32_t>(*is);
            for (auto i = 0; i < lod.nClipFlags; i++) {
                lod.clipFlags.push_back(val);
            }
        }
        else {
            //auto uncompressed = readLZOCompressed<uint32_t>(*is, (size_t)lod.nClipFlags * 4).first;
            auto uncompressed = readCompressed(*is, (size_t)lod.nClipFlags * 4, this->useCompression);
            for (auto& flag : uncompressed) {
                lod.clipFlags.push_back((ClipFlag)flag);
            }
        }
    }

    lod.defaultUvSet = readUVSet();

    lod.nUvs = readBytes<uint32_t>(*is);

    // 0 = default uv set
    for (auto i = 1; i < lod.nUvs; i++) {
        lod.uvSets.push_back(readUVSet());
    }

    lod.nPoints = readBytes<uint32_t>(*is);
    
    auto expectedSizeVertices = lod.nPoints * 12;
    std::vector<float_t> vertices = {};
    bool lzoCompressed = expectedSizeVertices >= 1024;
    if (useCompression) {
        lzoCompressed = readBytes<bool>(*is);
    }
    if (lzoCompressed) {
        vertices = readLZOCompressed<float_t>(*is, expectedSizeVertices).first;
    }
    else {
        for (auto i = 0; i < expectedSizeVertices / 4; i++) {
            vertices.push_back(readBytes<float_t>(*is));
        }
    }

    for (auto i = 0; i < vertices.size(); i+= 3) {
        lod.lodPoints.push_back({ vertices[i], vertices[i + 1], vertices[i + 2] });
    }

    return lod;
    for (auto i = 0; i < vertices.size(); i += 3) {
        VertProperty vertProperty;
        vertProperty.index = vertices[i];
        vertProperty.a = vertices[i + 1];
        vertProperty.b = vertices[i + 2];
        lod.vertProperties.push_back(vertProperty);
    }

    /*
    if (version >= 45) {
        lod.nNormals = readBytes<uint32_t>(*is);
        if (readBytes<bool>(*is)) {
            auto val = readBytes<uint32_t>(*is);
            auto xyz = decodeXYZ(val);
            for (auto i = 0; i < lod.nNormals; i++) {
                lod.lodNormals.push_back(xyz);
            }
        }
        else {
            //auto uncompressed = readLZOCompressed<uint32_t>(*is, (size_t)lod.nNormals * 4).first;
            auto uncompressed = readCompressedArray<uint32_t>(*is, (size_t)lod.nNormals * 4, useCompression, lod.nNormals);
            for (auto& compressedXYZ : uncompressed) {
               lod.lodNormals.push_back(decodeXYZ(compressedXYZ));
            }
        }
    }
    else {
        throw std::runtime_error("TODO implement");
    }

    */
    return lod;
    // TODO after rework

    if (version >= 45) {
        lod.nMinMax = readBytes<uint32_t>(*is);
        auto expectedSizeMinMax = lod.nMinMax * 8;
        std::vector<float_t> minMax = {};
        bool lzoCompressed = expectedSizeMinMax >= 1024;
        if (useCompression) {
            lzoCompressed = readBytes<bool>(*is);
        }
        if (lzoCompressed) {
            minMax = readLZOCompressed<float_t>(*is, expectedSizeMinMax).first;
        }
        else {
            for (auto i = 0; i < expectedSizeMinMax / 4; i++) {
                minMax.push_back(readBytes<float_t>(*is));
            }
        }
        for (auto i = 0; i < minMax.size(); i += 3) {
            XYZTriplet xyzTriplet;
            xyzTriplet[0] = vertices[i];
            xyzTriplet[1] = vertices[i + 1];
            xyzTriplet[2] = vertices[i + 2];
            lod.lodMinMax.push_back(xyzTriplet);
        }
    }
    else {
        throw std::runtime_error("TODO implement");
    }
    /*
    lod.nProperties = readBytes<uint32_t>(*is);
    auto expectedSizeProps = lod.nProperties * 12;
    std::vector<float_t> props = {};
    bool lzoCompressed = expectedSizeProps >= 1024;
    if (useCompression) {
        lzoCompressed = readBytes<bool>(*is);
    }
    if (lzoCompressed) {
        props = readLZOCompressed<float_t>(*is, expectedSizeProps).first;
    }
    else {
        for (auto i = 0; i < expectedSizeProps / 4; i++) {
            props.push_back(readBytes<float_t>(*is));
        }
    }
    for (auto i = 0; i < props.size(); i += 3) {
        XYZTriplet xyzTriplet;
        xyzTriplet[0] = vertices[i];
        xyzTriplet[1] = vertices[i + 1];
        xyzTriplet[2] = vertices[i + 2];
        lod.lodMinMax.push_back(xyzTriplet);
    }
    */

}

UVSet grad_aff::Odol::readUVSet() {
    UVSet uvSet;
    if (version >= 45) {
        uvSet.minU = readBytes<float_t>(*is);
        uvSet.minV = readBytes<float_t>(*is);
        uvSet.maxU = readBytes<float_t>(*is);
        uvSet.maxV = readBytes<float_t>(*is);
    }

    uvSet.nVertices = readBytes<uint32_t>(*is);
    uvSet.defaultFill = readBytes<bool>(*is);
    if (uvSet.defaultFill) {
        if (version >= 45) {
            uvSet.defaultValue = readBytes<float_t>(*is);
        }
        else {
            uvSet.defaultValue = std::make_pair<float_t, float_t>(readBytes<float_t>(*is), readBytes<float_t>(*is));
        }
        return uvSet;
    }
    else {
        uvSet.uvData = readCompressed(*is, (size_t)uvSet.nVertices * (version >= 45 ? 4 : 8), useCompression);
    }
    return uvSet;
}


XYZTriplet grad_aff::Odol::decodeXYZ(uint32_t CompressedXYZ)
{
    XYZTriplet triplet;
    double scaleFactor = -1.0 / 511;
    triplet[0] = triplet[1] = triplet[2] = 0.0;
    int x = CompressedXYZ & 0x3FF;
    int y = (CompressedXYZ >> 10) & 0x3FF;
    int z = (CompressedXYZ >> 20) & 0x3FF;
    if (x > 511) x -= 1024;
    if (y > 511) y -= 1024;
    if (z > 511) z -= 1024;
    if (x) triplet[0] = (float)(x * scaleFactor);
    if (y) triplet[1] = (float)(y * scaleFactor);
    if (x) triplet[2] = (float)(z * scaleFactor);
    return triplet;
}

std::vector<uint8_t> grad_aff::Odol::readLZOWithRule(uint32_t expectedSize)
{
    bool flag = expectedSize >= 1024;
    if (useCompression)
    {
        flag = readBytes<bool>(*is);
    }
    if (!flag)
    {
        return readBytes(*is, expectedSize);
    }
    return readLZOCompressed(*is, expectedSize).first;
}

LodType grad_aff::Odol::getLodType(float_t resolution) {
    auto result = lodMap.find(resolution);
    if (result != lodMap.end()) {
        return result->second;
    }
    else {
        return LodType::RESOLUTION;
    }
}