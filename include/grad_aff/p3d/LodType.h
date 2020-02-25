#pragma once

#include <math.h>
#include <map>

enum class LodType
{
    SPECIAL_LOD,
    GEOMETRY,
    BUOYANCY,
    PHYSXOLD,
    PHYSX,
    MEMORY,
    LANDCONTACT,
    ROADWAY,
    PATHS,
    HITPOINTS,
    VIEW_GEOMETRY,
    FIRE_GEOMETRY,
    VIEW_GEOMETRY_CARGO,
    VIEW_GEOMETRY_PILOT,
    VIEW_GEOMETRY_GUNNER,
    FIRE_GEOMETRY_GUNNER,
    SUBPARTS,
    SHADOWVOLUME_CARGO,
    SHADOWVOLUME_PILOT,
    SHADOWVOLUME_GUNNER,
    WRECK,
    VIEW_COMMANDER,
    VIEW_GUNNER,
    VIEW_PILOT,
    VIEW_CARGO,
    SHADOWVOLUME,
    SHADOWBUFFER,
    SHADOW_MIN,
    SHADOW_MAX,
    RESOLUTION
};

/*
class LodType {
    float f;
    LodType(float arg) : f(arg) {}
public:
    static const LodType SPECIAL_LOD;
    static const LodType GEOMETRY;
    operator float() const { return f; }
};
const LodType LodType::SPECIAL_LOD(1E+15f);
    /*const LodType LodType::GEOMETRY = 1E+13f,
    BUOYANCY = 2E+13f,
    PHYSXOLD = 3E+13f,
    PHYSX = 4E+13f,
    MEMORY = 1E+15f,
    LANDCONTACT = 2E+15f,
    ROADWAY = 3E+15f,
    PATHS = 4E+15f,
    HITPOINTS = 5E+15f,
    VIEW_GEOMETRY = 6E+15f,
    FIRE_GEOMETRY = 7E+15f,
    VIEW_GEOMETRY_CARGO = 8E+15f,
    VIEW_GEOMETRY_PILOT = 1.3E+16f,
    VIEW_GEOMETRY_GUNNER = 1.5E+16f,
    FIRE_GEOMETRY_GUNNER = 1.6E+16f,
    SUBPARTS = 1.7E+16f,
    SHADOWVOLUME_CARGO = 1.8E+16f,
    SHADOWVOLUME_PILOT = 1.9E+16f,
    SHADOWVOLUME_GUNNER = 2E+16f,
    WRECK = 2.1E+16f,
    VIEW_COMMANDER = 1E+16f,
    VIEW_GUNNER = 1000f,
    VIEW_PILOT = 1100f,
    VIEW_CARGO = 1200f,
    SHADOWVOLUME = 10000f,
    SHADOWBUFFER = 11000f,
    SHADOW_MIN = 10000f,
    SHADOW_MAX = 20000f
};*/