// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Utility\hdr_cubemap_generation.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Utility/hdr_cubemap_generation.h"

#include <cmath>

VESPERENGINE_NAMESPACE_BEGIN

namespace
{
    constexpr float PI = 3.14159265359f;

    glm::vec3 FaceDirection(uint32 face, float u, float v)
    {
        // Remap u,v to [-1, 1]
        const float a = 2.0f * u - 1.0f;
        const float b = 2.0f * v - 1.0f;

        // Flip for faces where +Y is "down" in UV layout
        switch (face)
        {
        case 0:  return glm::normalize(glm::vec3(1.0f, -b, -a));   // +X (right)
        case 1:  return glm::normalize(glm::vec3(-1.0f, -b, a));   // -X (left)
        case 2:  return glm::normalize(glm::vec3(a, 1.0f, b));     // +Y (top)
        case 3:  return glm::normalize(glm::vec3(a, -1.0f, -b));   // -Y (bottom)
        case 4:  return glm::normalize(glm::vec3(a, -b, 1.0f));    // +Z (front)
        default: return glm::normalize(glm::vec3(-a, -b, -1.0f));  // -Z (back)
        }
    }

    glm::vec2 DirectionToEquirectangularUV(const glm::vec3& dir)
    {
        float phi = std::atan2(dir.z, dir.x);
        float theta = std::acos(glm::clamp(dir.y, -1.0f, 1.0f));
        float u = (phi + PI) / (2.0f * PI);
        float v = theta / PI;
        return { u, v };
    }

    glm::vec2 DirectionToHemisphereUV(const glm::vec3& dir)
    {
        float phi = std::atan2(dir.z, dir.x);
        float theta = std::acos(glm::clamp(dir.y, 0.0f, 1.0f));
        float u = (phi + PI) / (2.0f * PI);
        float v = theta / (PI * 0.5f);
        return { u, v };
    }

    glm::vec2 DirectionToParabolicUV(const glm::vec3& dir)
    {
        glm::vec3 n = glm::normalize(dir);
        glm::vec2 uv;
        if (n.z >= 0.0f)
        {
            uv = glm::vec2(n.x, n.y) / (1.0f + n.z);
            uv = uv * 0.5f + 0.5f;
            uv.y *= 0.5f;
        }
        else
        {
            uv = glm::vec2(n.x, -n.y) / (1.0f - n.z);
            uv = uv * 0.5f + 0.5f;
            uv.y = uv.y * 0.5f + 0.5f;
        }
        return uv;
    }

    glm::vec3 SampleBilinear(const float* data,
        int32 width,
        int32 height,
        const glm::vec2& uv,
        bool wrapU = false)
    {
        float u = uv.x * static_cast<float>(width - 1);
        float v = glm::clamp(uv.y, 0.0f, 1.0f) * static_cast<float>(height - 1);
        int32 x0 = static_cast<int32>(std::floor(u));
        int32 y0 = static_cast<int32>(std::floor(v));
        int32 x1 = x0 + 1;
        int32 y1 = glm::min(y0 + 1, height - 1);

        if (wrapU)
        {
            auto mod = [&](int32 a, int32 b)
                {
                    int32 r = a % b;
                    return r < 0 ? r + b : r;
                };
            x0 = mod(x0, width);
            x1 = mod(x1, width);
        }
        else
        {
            x0 = glm::clamp(x0, 0, width - 1);
            x1 = glm::clamp(x1, 0, width - 1);
        }

        float fu = u - static_cast<float>(x0);
        float fv = v - static_cast<float>(y0);

        auto sample = [&](int32 x, int32 y)
            {
                const float* ptr = data + (y * width + x) * 4;
                return glm::vec3(ptr[0], ptr[1], ptr[2]);
            };

        glm::vec3 c00 = sample(x0, y0);
        glm::vec3 c10 = sample(x1, y0);
        glm::vec3 c01 = sample(x0, y1);
        glm::vec3 c11 = sample(x1, y1);

        glm::vec3 c0 = glm::mix(c00, c10, fu);
        glm::vec3 c1 = glm::mix(c01, c11, fu);
        return glm::mix(c0, c1, fv);
    }

    glm::vec3 Sample(const float* data,
        int32 width,
        int32 height,
        const glm::vec2& uv,
        bool wrapU = false)
    {
        return SampleBilinear(data, width, height, uv, wrapU);
    }

    uint16_t Float32ToFloat16(float value)
    {
        uint32_t bits = *reinterpret_cast<uint32_t*>(&value);
        uint32_t sign = (bits >> 31) & 0x1;
        int32_t exp = ((bits >> 23) & 0xFF) - 127 + 15;
        uint32_t mant = bits & 0x7FFFFF;

        if (exp <= 0)
        {
            return static_cast<uint16_t>(sign << 15);
        }
        else if (exp >= 31)
        {
            return static_cast<uint16_t>((sign << 15) | 0x7C00);
        }
        return static_cast<uint16_t>((sign << 15) | (exp << 10) | (mant >> 13));
    }
}

HDRProjectionType HDRCubemapCPU::DetectHDRProjectionType(int32 width, int32 height)
{
    auto isCubeCross = [&](int32 w, int32 h)
        {
            bool vertical = (w / 3 == h / 4) && (w % 3 == 0) && (h % 4 == 0);
            bool horizontal = (w / 4 == h / 3) && (w % 4 == 0) && (h % 3 == 0);
            return vertical || horizontal;
        };

    auto isHemisphere = [&](int32 w, int32 h)
        {
            return w == h;
        };

    constexpr float ASPECT_EPS = 0.05f;

    auto isEquirectangular = [&](int32 w, int32 h)
        {
            float aspect = static_cast<float>(w) / static_cast<float>(h);
            return std::fabsf(aspect - 2.0f) < ASPECT_EPS;
        };

    auto isParabolic = [&](int32 w, int32 h)
        {
            if (w % 2 != 0 || h % 2 != 0) return false;
            int32 halfHeight = h / 2;
            int32 halfWidth = w / 2;
            return std::fabsf(static_cast<float>(w) / h - 2.0f) < ASPECT_EPS &&
                (halfWidth == halfHeight);
        };

    if (isCubeCross(width, height))
        return HDRProjectionType::Cube;
    if (isEquirectangular(width, height))
        return HDRProjectionType::LatLongCubemap;
    if (isParabolic(width, height))
        return HDRProjectionType::Parabolic;
    if (isHemisphere(width, height))
        return HDRProjectionType::Hemisphere;

    return HDRProjectionType::Equirectangular;
}


std::vector<float> HDRCubemapCPU::GenerateFloat32Cubemap(const float* hdrData,
    int32 width,
    int32 height,
    uint32 faceSize,
    HDRProjectionType projection)
{
    const uint64 facePixels = static_cast<uint64>(faceSize) * faceSize;
    std::vector<float> result(facePixels * 6 * 4, 1.0f);

    uint32 cubeSize = faceSize;
    for (uint32 face = 0; face < 6; ++face)
    {
        for (uint32 y = 0; y < cubeSize; ++y)
        {
            for (uint32 x = 0; x < cubeSize; ++x)
            {
                float u = (x + 0.5f) / static_cast<float>(cubeSize);
                float v = (y + 0.5f) / static_cast<float>(cubeSize);
                glm::vec3 dir = FaceDirection(face, u, v);
                glm::vec2 uv;
                if (projection == HDRProjectionType::Equirectangular || projection == HDRProjectionType::LatLongCubemap)
                    uv = DirectionToEquirectangularUV(dir);
                else if (projection == HDRProjectionType::Hemisphere)
                    uv = DirectionToHemisphereUV(dir);
                else if (projection == HDRProjectionType::Parabolic)
                    uv = DirectionToParabolicUV(dir);
                else // Cube cross
                {
                    // Map directly using cross layout
                    bool vertical = (width / 3 == height / 4);
                    uint32 s = vertical ? width / 3 : width / 4;
                    uint32 ox = 0, oy = 0;
                    if (vertical)
                    {
                        switch (face)
                        {
                        case 0: ox = 2 * s; oy = 1 * s; break; // +X
                        case 1: ox = 0 * s; oy = 1 * s; break; // -X
                        case 2: ox = 1 * s; oy = 0 * s; break; // +Y
                        case 3: ox = 1 * s; oy = 2 * s; break; // -Y
                        case 4: ox = 1 * s; oy = 1 * s; break; // +Z
                        case 5: ox = 1 * s; oy = 3 * s; break; // -Z
                        }
                    }
                    else
                    {
                        switch (face)
                        {
                        case 0: ox = 2 * s; oy = 1 * s; break; // +X
                        case 1: ox = 0 * s; oy = 1 * s; break; // -X
                        case 2: ox = 1 * s; oy = 0 * s; break; // +Y
                        case 3: ox = 1 * s; oy = 2 * s; break; // -Y
                        case 4: ox = 1 * s; oy = 1 * s; break; // +Z
                        case 5: ox = 3 * s; oy = 1 * s; break; // -Z
                        }
                    }
                    glm::vec2 uvCross;
                    uvCross.x = (static_cast<float>(ox) + u * static_cast<float>(s)) / static_cast<float>(width);
                    uvCross.y = (static_cast<float>(oy) + v * static_cast<float>(s)) / static_cast<float>(height);
                    glm::vec3 color = SampleBilinear(hdrData, width, height, uvCross);
                    size_t index = (face * facePixels + y * cubeSize + x) * 4;
                    result[index] = color.r;
                    result[index + 1] = color.g;
                    result[index + 2] = color.b;
                    continue;
                }

                bool wrap = (projection == HDRProjectionType::Equirectangular || projection == HDRProjectionType::LatLongCubemap);
                glm::vec3 color = Sample(hdrData, width, height, uv, wrap);
                size_t index = (face * facePixels + y * cubeSize + x) * 4;
                result[index] = color.r;
                result[index + 1] = color.g;
                result[index + 2] = color.b;
            }
        }
    }
    return result;
}

std::vector<uint16_t> HDRCubemapCPU::GenerateFloat16Cubemap(const float* hdrData,
    int32 width,
    int32 height,
    uint32 faceSize,
    HDRProjectionType projection)
{
    std::vector<float> full = GenerateFloat32Cubemap(hdrData, width, height, faceSize, projection);
    std::vector<uint16_t> half(full.size());
    for (size_t i = 0; i < full.size(); ++i)
    {
        half[i] = Float32ToFloat16(full[i]);
    }
    return half;
}

VESPERENGINE_NAMESPACE_END
