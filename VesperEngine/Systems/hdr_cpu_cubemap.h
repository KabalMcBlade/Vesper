#pragma once

#include "Core/core_defines.h"
#include "Core/glm_config.h"
#include <cstdint>
#include <vector>

VESPERENGINE_NAMESPACE_BEGIN

enum class HDRProjectionType : uint8
{
    Equirectangular,
    Cube,
    Hemisphere,
    Parabolic,
    LatLongCubemap
};

namespace HDRCubemapCPU
{
    HDRProjectionType DetectHDRProjectionType(int32 width, int32 height);

    std::vector<float> GenerateFloatCubemap(const float* hdrData,
                                            int32 width,
                                            int32 height,
                                            uint32 faceSize,
                                            HDRProjectionType projection);

    std::vector<uint16_t> GenerateHalfCubemap(const float* hdrData,
                                              int32 width,
                                              int32 height,
                                              uint32 faceSize,
                                              HDRProjectionType projection);
}

VESPERENGINE_NAMESPACE_END
