#include "Systems/hdr_cpu_cubemap.h"

#include <cmath>

VESPERENGINE_NAMESPACE_BEGIN

namespace
{
    constexpr float PI = 3.14159265359f;

    glm::vec3 FaceDirection(uint32 face, float u, float v)
    {
        const float a = 2.0f * u - 1.0f;
        const float b = 2.0f * v - 1.0f;
        switch (face)
        {
        case 0: return glm::normalize(glm::vec3(1.0f, -b, -a));
        case 1: return glm::normalize(glm::vec3(-1.0f, -b, a));
        case 2: return glm::normalize(glm::vec3(a, 1.0f, b));
        case 3: return glm::normalize(glm::vec3(a, -1.0f, -b));
        case 4: return glm::normalize(glm::vec3(a, -b, 1.0f));
        default: return glm::normalize(glm::vec3(-a, -b, -1.0f));
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
        glm::vec2 uv = n.xy / (1.0f + n.z);
        uv = uv * 0.5f + 0.5f;
        return uv;
    }

    glm::vec3 Sample(const float* data, int32 width, int32 height, const glm::vec2& uv)
    {
        float u = glm::clamp(uv.x, 0.0f, 1.0f);
        float v = glm::clamp(uv.y, 0.0f, 1.0f);
        int x = static_cast<int>(u * (width - 1));
        int y = static_cast<int>((1.0f - v) * (height - 1));
        const float* ptr = data + (y * width + x) * 4;
        return { ptr[0], ptr[1], ptr[2] };
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
    if ((width / 3 == height / 4) && (width % 3 == 0) && (height % 4 == 0))
        return HDRProjectionType::Cube;
    if ((width / 4 == height / 3) && (width % 4 == 0) && (height % 3 == 0))
        return HDRProjectionType::Cube;

    float aspect = static_cast<float>(width) / static_cast<float>(height);
    if (std::fabsf(aspect - 2.0f) < 0.00001f)
    {
        if (height % 2 == 0)
            return HDRProjectionType::Parabolic;
        return HDRProjectionType::Equirectangular;
    }

    if (width == height)
        return HDRProjectionType::Hemisphere;

    return HDRProjectionType::Equirectangular;
}

std::vector<float> HDRCubemapCPU::GenerateFloatCubemap(const float* hdrData,
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
                    uint32 sx = static_cast<uint32>(u * (s - 1));
                    uint32 sy = static_cast<uint32>(v * (s - 1));
                    const float* ptr = hdrData + ((oy + sy) * width + (ox + sx)) * 4;
                    size_t index = (face * facePixels + y * cubeSize + x) * 4;
                    result[index] = ptr[0];
                    result[index + 1] = ptr[1];
                    result[index + 2] = ptr[2];
                    continue;
                }

                glm::vec3 color = Sample(hdrData, width, height, uv);
                size_t index = (face * facePixels + y * cubeSize + x) * 4;
                result[index] = color.r;
                result[index + 1] = color.g;
                result[index + 2] = color.b;
            }
        }
    }
    return result;
}

std::vector<uint16_t> HDRCubemapCPU::GenerateHalfCubemap(const float* hdrData,
                                                         int32 width,
                                                         int32 height,
                                                         uint32 faceSize,
                                                         HDRProjectionType projection)
{
    std::vector<float> full = GenerateFloatCubemap(hdrData, width, height, faceSize, projection);
    std::vector<uint16_t> half(full.size());
    for (size_t i = 0; i < full.size(); ++i)
    {
        half[i] = Float32ToFloat16(full[i]);
    }
    return half;
}

VESPERENGINE_NAMESPACE_END
