#pragma once

#include <vector>
#include "../Types.h"

enum class UVSource
{
	UVNone,
	UVTex,
	UVTexWaterAnim,
	UVPos,
	UVNorm,
	UVTex1,
	UVWorldPos,
	UVWorldNorm,
	UVTexShoreAnim,
	NUVSource
};

struct LodStageTransform
{
	UVSource uvSource;
    TransformMatrix transFormMatrix = {};
};