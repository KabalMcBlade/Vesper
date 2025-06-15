// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Utility\hdr_cubemap_generation.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

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

    std::vector<float> GenerateFloat32Cubemap(const float* hdrData,
        int32 width,
        int32 height,
        uint32 faceSize,
        HDRProjectionType projection);

    std::vector<uint16_t> GenerateFloat16Cubemap(const float* hdrData,
        int32 width,
        int32 height,
        uint32 faceSize,
        HDRProjectionType projection);
}

VESPERENGINE_NAMESPACE_END
