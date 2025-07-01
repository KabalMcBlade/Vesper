// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Utility\gltf_loader.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Utility/gltf_loader.h"
#include "Utility/hash.h"
#include "Utility/logger.h"

#include "Core/glm_config.h"

#include "App/vesper_app.h"
#include "App/config.h"
#include "App/file_system.h"

#include "Backend/device.h"
#include "Backend/buffer.h"
#include "Backend/model_data.h"

#include "Systems/material_system.h"
#include "Systems/texture_system.h"

#include "stb/stb_image.h"

#define __STDC_LIB_EXT1__					// for using sprintf_s
#define TINYGLTF_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ThirdParty/include/tiny_gltf.h"

#include <stdexcept>
#include <unordered_map>
#include <array>

VESPERENGINE_NAMESPACE_BEGIN

namespace 
{
    template <typename T>
    void ReadAccessor(const tinygltf::Model& _model, const tinygltf::Accessor& _accessor, std::vector<T>& _out)
    {
        const tinygltf::BufferView& bufferView = _model.bufferViews[_accessor.bufferView];
        const tinygltf::Buffer& buffer = _model.buffers[bufferView.buffer];

        const size_t stride = _accessor.ByteStride(bufferView);
        const uint8* dataPtr = buffer.data.data() + bufferView.byteOffset + _accessor.byteOffset;

        _out.resize(_accessor.count);
        for (size_t i = 0; i < _accessor.count; ++i)
        {
            const uint8* src = dataPtr + stride * i;
            if constexpr (std::is_same_v<T, glm::vec3>)
            {
                const float* v = reinterpret_cast<const float*>(src);
                _out[i] = glm::vec3(v[0], v[1], v[2]);
            }
            else if constexpr (std::is_same_v<T, glm::vec4>)
            {
                const float* v = reinterpret_cast<const float*>(src);
                _out[i] = glm::vec4(v[0], v[1], v[2], v[3]);
            }
            else if constexpr (std::is_same_v<T, glm::vec2>)
            {
                const float* v = reinterpret_cast<const float*>(src);
                _out[i] = glm::vec2(v[0], v[1]);
            }
            else if constexpr (std::is_same_v<T, uint32>)
            {
                if (_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                {
                    const uint8* val = reinterpret_cast<const uint8*>(src);
                    _out[i] = static_cast<uint32>(*val);
                }
                else if (_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                {
                    const uint16* val = reinterpret_cast<const uint16*>(src);
                    _out[i] = static_cast<uint32>(*val);
                }
                else
                {
                    const uint32* val = reinterpret_cast<const uint32*>(src);
                    _out[i] = *val;
                }
            }
            else if constexpr (std::is_same_v<T, float>)
            {
                const float* val = reinterpret_cast<const float*>(src);
                _out[i] = *val;
            }
        }
    }

    void ReadColorAccessor(const tinygltf::Model& _model, const tinygltf::Accessor& _accessor, std::vector<glm::vec3>& _out) 
    {
        const tinygltf::BufferView& bufferView = _model.bufferViews[_accessor.bufferView];
        const tinygltf::Buffer& buffer = _model.buffers[bufferView.buffer];

        const size_t stride = _accessor.ByteStride(bufferView);
        const uint8* dataPtr = buffer.data.data() + bufferView.byteOffset + _accessor.byteOffset;

        _out.resize(_accessor.count);
        for (size_t i = 0; i < _accessor.count; ++i)
        {
            const uint8* src = dataPtr + stride * i;
            if (_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
            {
                const uint8* v = reinterpret_cast<const uint8*>(src);
                _out[i] = glm::vec3(v[0], v[1], v[2]) / 255.0f;
            }
            else if (_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
            {
                const uint16* v = reinterpret_cast<const uint16*>(src);
                _out[i] = glm::vec3(v[0], v[1], v[2]) / 65535.0f;
            }
            else 
            {
                const float* v = reinterpret_cast<const float*>(src);
                _out[i] = glm::vec3(v[0], v[1], v[2]);
            }
        }
    }

    std::shared_ptr<TextureData> GetTexture(const tinygltf::Model& _model, int32 _texIndex, const std::string& _modelPath, const std::string& _texturePath, MaterialSystem& _materialSystem)
    {
        if (_texIndex < 0 || _texIndex >= static_cast<int32>(_model.textures.size()))
        {
            return nullptr;
        }

        const tinygltf::Texture& texture = _model.textures[_texIndex];
        if (texture.source < 0 || texture.source >= static_cast<int32>(_model.images.size()))
        {
            return nullptr;
        }

        const tinygltf::Image& image = _model.images[texture.source];

        TextureSystem& textureSystem = _materialSystem.GetTextureSystem();

        if (!image.uri.empty() && image.uri.find("data:") != 0) 
        {
            std::string textureFullPath = _modelPath + image.uri;
            if (!FileSystem::IsFileExists(textureFullPath))
            {
                textureFullPath = _texturePath + image.uri;
            }

            int32 w, h, c;
            stbi_uc* img = stbi_load(textureFullPath.c_str(), &w, &h, &c, STBI_rgb_alpha);
            if (!img)
            {
                LOG(Logger::WARNING, "Failed to load texture: ", image.uri);
                return nullptr;
            }

            auto tex = textureSystem.LoadTexture(image.uri, VK_FORMAT_R8G8B8A8_SRGB, img, w, h);
            stbi_image_free(img);
            return tex;
        }

        if (!image.image.empty()) 
        {
            VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
            std::vector<uint8> data;

            if (image.bits == 8) 
            {
                if (image.component == 4) 
                {
                    data.assign(image.image.begin(), image.image.end());
                }
                else if (image.component == 3) 
                {
                    size_t pixelCount = static_cast<size_t>(image.width) * image.height;
                    data.resize(pixelCount * 4);
                    for (size_t i = 0; i < pixelCount; ++i)
                    {
                        data[i * 4 + 0] = image.image[i * 3 + 0];
                        data[i * 4 + 1] = image.image[i * 3 + 1];
                        data[i * 4 + 2] = image.image[i * 3 + 2];
                        data[i * 4 + 3] = 255;
                    }
                }
                else if (image.component == 1)
                {
                    format = VK_FORMAT_R8_UNORM;
                    data.assign(image.image.begin(), image.image.end());
                }
                else 
                {
                    LOG(Logger::WARNING, "Unsupported component count for image: ", image.name);
                    return nullptr;
                }
                std::string texName = image.name.empty() ? ("embedded_" + std::to_string(texture.source)) : image.name;
                return textureSystem.LoadTexture(texName, format, data.data(), image.width, image.height);
            }
        }

        LOG(Logger::WARNING, "Embedded image not supported: ", image.name);
        return nullptr;
    }

    void GenerateTangents(const std::vector<glm::vec3>& _positions,
        const std::vector<glm::vec3>& _normals,
        const std::vector<glm::vec2>& _uvs,
        const std::vector<uint32>& _indices,
        std::vector<glm::vec4>& _outTangents)
    {
        size_t count = _positions.size();
        _outTangents.assign(count, glm::vec4(0.0f));
        if (_positions.empty() || _normals.empty() || _uvs.empty())
            return;

        std::vector<glm::vec3> tan1(count, glm::vec3(0.0f));
        std::vector<glm::vec3> tan2(count, glm::vec3(0.0f));

        for (size_t i = 0; i + 2 < _indices.size(); i += 3)
        {
            uint32 i1 = _indices[i];
            uint32 i2 = _indices[i + 1];
            uint32 i3 = _indices[i + 2];

            const glm::vec3& v1 = _positions[i1];
            const glm::vec3& v2 = _positions[i2];
            const glm::vec3& v3 = _positions[i3];

            const glm::vec2& w1 = _uvs[i1];
            const glm::vec2& w2 = _uvs[i2];
            const glm::vec2& w3 = _uvs[i3];

            float x1 = v2.x - v1.x;
            float x2 = v3.x - v1.x;
            float y1 = v2.y - v1.y;
            float y2 = v3.y - v1.y;
            float z1 = v2.z - v1.z;
            float z2 = v3.z - v1.z;

            float s1 = w2.x - w1.x;
            float s2 = w3.x - w1.x;
            float t1 = w2.y - w1.y;
            float t2 = w3.y - w1.y;

            float r = (s1 * t2 - s2 * t1);
            if (r != 0.0f) r = 1.0f / r;

            glm::vec3 sdir((t2 * x1 - t1 * x2) * r,
                (t2 * y1 - t1 * y2) * r,
                (t2 * z1 - t1 * z2) * r);
            glm::vec3 tdir((s1 * x2 - s2 * x1) * r,
                (s1 * y2 - s2 * y1) * r,
                (s1 * z2 - s2 * z1) * r);

            tan1[i1] += sdir;
            tan1[i2] += sdir;
            tan1[i3] += sdir;

            tan2[i1] += tdir;
            tan2[i2] += tdir;
            tan2[i3] += tdir;
        }

        for (size_t i = 0; i < count; ++i)
        {
            const glm::vec3& n = _normals[i];
            const glm::vec3& t = tan1[i];

            glm::vec3 tangent = glm::normalize(t - n * glm::dot(n, t));
            float w = (glm::dot(glm::cross(n, t), tan2[i]) < 0.0f) ? -1.0f : 1.0f;
            _outTangents[i] = glm::vec4(tangent, w);
        }
    }

    glm::mat4 ComposeTransform(const tinygltf::Node& _node)
    {
        glm::mat4 transform(1.0f);

        if (_node.matrix.size() == 16)
        {
            transform = glm::make_mat4x4(_node.matrix.data());
        }
        else
        {
            if (_node.translation.size() == 3)
            {
                transform = glm::translate(transform, glm::vec3(
                    _node.translation[0], _node.translation[1], _node.translation[2]));
            }

            if (_node.rotation.size() == 4)
            {
                glm::quat rot(static_cast<float>(_node.rotation[3]), static_cast<float>(_node.rotation[0]), static_cast<float>(_node.rotation[1]), static_cast<float>(_node.rotation[2]));
                transform *= glm::mat4_cast(rot);
            }

            if (_node.scale.size() == 3)
            {
                transform = glm::scale(transform, glm::vec3(
                    _node.scale[0], _node.scale[1], _node.scale[2]));
            }
        }

        return transform;
    }

    void LoadAnimations(const tinygltf::Model& _model, std::vector<MorphAnimation>& _animations)
    {
        for (const auto& anim : _model.animations)
        {
            MorphAnimation animation{};
            animation.Name = anim.name;

            for (const auto& channel : anim.channels)
            {
                if (channel.target_path != "weights")
                    continue;

                if (channel.sampler < 0 || channel.sampler >= static_cast<int32>(anim.samplers.size()))
                    continue;

                const auto& sampler = anim.samplers[channel.sampler];
                std::vector<float> times;
                std::vector<float> values;
                ReadAccessor(_model, _model.accessors[sampler.input], times);
                ReadAccessor(_model, _model.accessors[sampler.output], values);

                size_t keyCount = times.size();
                if (keyCount == 0)
                    continue;

                size_t morphCount = values.size() / keyCount;
                animation.Keyframes.resize(keyCount);
                for (size_t i = 0; i < keyCount; ++i)
                {
                    animation.Keyframes[i].Time = times[i];
                    for (glm::vec<4, float, glm::aligned_highp>::length_type m = 0; m < morphCount && m < kMaxMorphTargets; ++m)
                    {
                        float v = values[i * morphCount + m];
                        if (m < 4)
                        {
                            animation.Keyframes[i].Weights[0][m] = v;
                        }
                        else
                        {
                            animation.Keyframes[i].Weights[1][m - 4] = v;
                        }
                    }
                }
            }

            if (!animation.Keyframes.empty())
            {
                _animations.push_back(std::move(animation));
            }
        }
    }

    std::unique_ptr<ModelData> LoadPrimitiveModel(const tinygltf::Model& _gltfModel,
        const tinygltf::Primitive& _primitive, const glm::mat4& _transform, const std::array<glm::vec4, 2>& _weights, const bool _isMirrored,
        const std::string& _basePath, const std::string& _texturePath, MaterialSystem& _materialSystem, bool _isStatic)
    {
        auto modelData = std::make_unique<ModelData>();

        glm::vec3 baseColorFactorRGB(1.0f);
        float baseAlphaFactor = 1.0f;
        float alphaCutoff = -1.0f;
        bool isTransparent = false;
        bool isDoubleSided = false;

        if (_primitive.material >= 0 && _primitive.material < static_cast<int32>(_gltfModel.materials.size()))
        {
            const tinygltf::Material& gltfMaterial = _gltfModel.materials[_primitive.material];
            const tinygltf::PbrMetallicRoughness& pbr = gltfMaterial.pbrMetallicRoughness;

            auto roughTex = GetTexture(_gltfModel, pbr.metallicRoughnessTexture.index, _basePath, _texturePath, _materialSystem);
            auto metallicTex = roughTex;


            // Optional PBR values
            float sheenFactor = 0.0f;
            float clearcoatFactor = 0.0f;
            float clearcoatRoughness = 0.0f;
            float anisotropy = 0.0f;
            float anisotropyRotation = 0.0f;
            std::shared_ptr<TextureData> sheenTex = nullptr;
            int32 sheenTexCoord = 0;

            const auto sheenIt = gltfMaterial.extensions.find("KHR_materials_sheen");
            if (sheenIt != gltfMaterial.extensions.end() && sheenIt->second.IsObject())
            {
                const auto& sheenVal = sheenIt->second;
                if (sheenVal.Has("sheenColorFactor"))
                {
                    const auto& arr = sheenVal.Get("sheenColorFactor");
                    if (arr.IsArray() && arr.ArrayLen() > 0)
                    {
                        sheenFactor = static_cast<float>(arr.Get(0).GetNumberAsDouble());
                    }
                }
                if (sheenVal.Has("sheenColorTexture"))
                {
                    const auto& texObj = sheenVal.Get("sheenColorTexture");
                    if (texObj.IsObject())
                    {
                        int texIndex = -1;
                        if (texObj.Get("index").IsNumber())
                        {
                            texIndex = texObj.Get("index").GetNumberAsInt();
                        }
                        if (texObj.Has("texCoord"))
                        {
                            sheenTexCoord = texObj.Get("texCoord").GetNumberAsInt();
                        }
                        sheenTex = GetTexture(_gltfModel, texIndex, _basePath, _texturePath, _materialSystem);
                    }
                }
            }

            const auto clearcoatIt = gltfMaterial.extensions.find("KHR_materials_clearcoat");
            if (clearcoatIt != gltfMaterial.extensions.end() && clearcoatIt->second.IsObject())
            {
                const auto& cc = clearcoatIt->second;
                if (cc.Has("clearcoatFactor"))
                {
                    clearcoatFactor = static_cast<float>(cc.Get("clearcoatFactor").GetNumberAsDouble());
                }
                if (cc.Has("clearcoatRoughnessFactor"))
                {
                    clearcoatRoughness = static_cast<float>(cc.Get("clearcoatRoughnessFactor").GetNumberAsDouble());
                }
            }

            const auto anisoIt = gltfMaterial.extensions.find("KHR_materials_anisotropy");
            if (anisoIt != gltfMaterial.extensions.end() && anisoIt->second.IsObject())
            {
                const auto& an = anisoIt->second;
                if (an.Has("anisotropyStrength"))
                {
                    anisotropy = static_cast<float>(an.Get("anisotropyStrength").GetNumberAsDouble());
                }
                if (an.Has("anisotropyRotation"))
                {
                    anisotropyRotation = static_cast<float>(an.Get("anisotropyRotation").GetNumberAsDouble());
                }
            }

            isTransparent = gltfMaterial.alphaMode == "BLEND";
            alphaCutoff = gltfMaterial.alphaMode == "MASK" ? static_cast<float>(gltfMaterial.alphaCutoff) : -1.0f;
			isDoubleSided = gltfMaterial.doubleSided;

            std::array<float, 4> baseColorDefault = { 1.0f, 1.0f, 1.0f, 1.0f };
            for (size_t i = 0; i < pbr.baseColorFactor.size() && i < 4; ++i)
            {
                baseColorDefault[i] = static_cast<float>(pbr.baseColorFactor[i]);
            }

            baseColorFactorRGB = glm::vec3(baseColorDefault[0], baseColorDefault[1], baseColorDefault[2]);
            baseAlphaFactor = baseColorDefault[3];

            auto albedoTex = GetTexture(_gltfModel, pbr.baseColorTexture.index, _basePath, _texturePath, _materialSystem);
            auto occlusionTex = GetTexture(_gltfModel, gltfMaterial.occlusionTexture.index, _basePath, _texturePath, _materialSystem);
            auto emissiveTex = GetTexture(_gltfModel, gltfMaterial.emissiveTexture.index, _basePath, _texturePath, _materialSystem);
            auto normalTex = GetTexture(_gltfModel, gltfMaterial.normalTexture.index, _basePath, _texturePath, _materialSystem);

            std::vector<int32> uvIndices = {
                pbr.metallicRoughnessTexture.texCoord,
                pbr.metallicRoughnessTexture.texCoord,
                sheenTexCoord,
                gltfMaterial.emissiveTexture.texCoord,
                gltfMaterial.normalTexture.texCoord,
                pbr.baseColorTexture.texCoord,
                gltfMaterial.occlusionTexture.texCoord
            };

            modelData->Material = _materialSystem.CreateMaterial(
                gltfMaterial.name,
                { roughTex, metallicTex, sheenTex, emissiveTex, normalTex, albedoTex, occlusionTex },
                { static_cast<float>(pbr.roughnessFactor),
                    static_cast<float>(pbr.metallicFactor), sheenFactor,
                    clearcoatFactor, clearcoatRoughness,
                    anisotropy, anisotropyRotation,
                    alphaCutoff, baseAlphaFactor },
                isTransparent, isDoubleSided, MaterialType::PBR, uvIndices);

			modelData->IsMirrored = _isMirrored;
        }
        else
        {
            std::vector<std::shared_ptr<vesper::TextureData>> emptyTextures = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
            std::vector<std::any> emptyMetadata = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f };

            modelData->Material = _materialSystem.CreateMaterial(
                "_DefaultGltfPBR_", emptyTextures, emptyMetadata, isTransparent, isDoubleSided,
                MaterialType::PBR, {});

            modelData->IsMirrored = _isMirrored;
        }

        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs1;
        std::vector<glm::vec2> uvs2;
        std::vector<glm::vec3> colors;

        auto itPos = _primitive.attributes.find("POSITION");
        if (itPos != _primitive.attributes.end())
        {
            ReadAccessor(_gltfModel, _gltfModel.accessors[itPos->second], positions);
        }

        auto itNorm = _primitive.attributes.find("NORMAL");
        if (itNorm != _primitive.attributes.end())
        {
            ReadAccessor(_gltfModel, _gltfModel.accessors[itNorm->second], normals);
        }

        auto itUV1 = _primitive.attributes.find("TEXCOORD_0");
        if (itUV1 != _primitive.attributes.end())
        {
            ReadAccessor(_gltfModel, _gltfModel.accessors[itUV1->second], uvs1);
        }

        auto itUV2 = _primitive.attributes.find("TEXCOORD_1");
        if (itUV2 != _primitive.attributes.end())
        {
            ReadAccessor(_gltfModel, _gltfModel.accessors[itUV2->second], uvs2);
        }

        auto itColor = _primitive.attributes.find("COLOR_0");
        if (itColor != _primitive.attributes.end())
        {
            ReadColorAccessor(_gltfModel, _gltfModel.accessors[itColor->second], colors);
        }

        size_t morphCount = std::min<size_t>(_primitive.targets.size(), kMaxMorphTargets);
        std::vector<std::vector<glm::vec3>> morphPositions(morphCount);
        std::vector<std::vector<glm::vec3>> morphNormals(morphCount);
        for (size_t t = 0; t < morphCount; ++t)
        {
            auto mp = _primitive.targets[t].find("POSITION");
            if (mp != _primitive.targets[t].end())
            {
                ReadAccessor(_gltfModel, _gltfModel.accessors[mp->second], morphPositions[t]);
            }

            auto mn = _primitive.targets[t].find("NORMAL");
            if (mn != _primitive.targets[t].end())
            {
                ReadAccessor(_gltfModel, _gltfModel.accessors[mn->second], morphNormals[t]);
            }
        }

        std::vector<uint32> indices;
        if (_primitive.indices >= 0)
        {
            ReadAccessor(_gltfModel, _gltfModel.accessors[_primitive.indices], indices);
        }
        else
        {
            indices.resize(positions.size());
            for (uint32 i = 0; i < indices.size(); ++i)
            {
                indices[i] = i;
            }
        }

        std::vector<glm::vec4> tangents;
        auto itTan = _primitive.attributes.find("TANGENT");
        if (itTan != _primitive.attributes.end())
        {
            ReadAccessor(_gltfModel, _gltfModel.accessors[itTan->second], tangents);
        }

        if (tangents.empty())
        {
            const std::vector<glm::vec2>& uvSource = !uvs1.empty() ? uvs1 : uvs2;
            GenerateTangents(positions, normals, uvSource, indices, tangents);
        }

        modelData->Vertices.resize(indices.size());
        modelData->Indices.resize(indices.size());
        modelData->MorphTargetCount = static_cast<uint32>(morphCount);
        modelData->MorphWeights[0] = _weights[0];
        modelData->MorphWeights[1] = _weights[1];

        glm::mat3 normalMatrix = glm::mat3(_transform);

        for (size_t i = 0; i < indices.size(); ++i)
        {
            uint32 vIndex = indices[i];
            Vertex vertex{};
            if (vIndex < positions.size())
            {
                glm::vec4 pos = _transform * glm::vec4(positions[vIndex], 1.0f);
                vertex.Position = glm::vec3(pos);
            }
            if (vIndex < normals.size())
            {
                vertex.Normal = normalMatrix * normals[vIndex];
            }
            if (vIndex < uvs1.size())
            {
                vertex.UV1 = uvs1[vIndex];
            }
            if (vIndex < uvs2.size())
            {
                vertex.UV2 = uvs2[vIndex];
            }
            if (vIndex < tangents.size())
            {
                glm::vec4 t = tangents[vIndex];
                vertex.Tangent = glm::vec4(normalMatrix * glm::vec3(t), t.w * (_isMirrored ? -1.0f : 1.0f));
            }
            if (vIndex < colors.size())
            {
                vertex.Color = colors[vIndex] * baseColorFactorRGB;
            }
            else
            {
                vertex.Color = baseColorFactorRGB;
            }

            for (size_t t = 0; t < morphCount; ++t)
            {
                if (vIndex < morphPositions[t].size())
                {
                    vertex.MorphPos[t] = glm::mat3(_transform) * morphPositions[t][vIndex];
                }
                if (vIndex < morphNormals[t].size())
                {
                    vertex.MorphNorm[t] = normalMatrix * morphNormals[t][vIndex];
                }
            }

            modelData->Vertices[i] = vertex;
            modelData->Indices[i] = static_cast<uint32>(i);
        }

        modelData->IsStatic = _isStatic;
        return modelData;
    }

    void ProcessNode(const tinygltf::Model& _gltfModel, int32 _nodeIdx, const glm::mat4& _parent,
        std::vector<std::unique_ptr<ModelData>>& _models, bool _isStatic,
        const std::string& _basePath, const std::string& _texturePath, MaterialSystem& _materialSystem)
    {
        const tinygltf::Node& node = _gltfModel.nodes[_nodeIdx];
        glm::mat4 transform = _parent * ComposeTransform(node);

        // Determine if the resulting transform is mirrored (left-handed)
        const glm::vec3 x = glm::vec3(transform[0]);
        const glm::vec3 y = glm::vec3(transform[1]);
        const glm::vec3 z = glm::vec3(transform[2]);
        const bool isMirrored = glm::dot(glm::cross(x, y), z) < 0.0f;

        std::array<glm::vec4, 2> weights{ glm::vec4(0.0f), glm::vec4(0.0f) };
        bool hasNodeWeights = false;

        if (!node.weights.empty())
        {
            for (size_t i = 0; i < node.weights.size() && i < kMaxMorphTargets; ++i)
            {
                weights[i / 4][i % 4] = static_cast<float>(node.weights[i]);
                hasNodeWeights = true;
            }
        }

        if (node.mesh >= 0 && node.mesh < static_cast<int32>(_gltfModel.meshes.size()))
        {
            const tinygltf::Mesh& mesh = _gltfModel.meshes[node.mesh];

            if (!hasNodeWeights && !mesh.weights.empty())
            {
                for (size_t i = 0; i < mesh.weights.size() && i < kMaxMorphTargets; ++i)
                {
                    weights[i / 4][i % 4] = static_cast<float>(mesh.weights[i]);
                }
            }

            for (const auto& primitive : mesh.primitives)
            {
                auto model = LoadPrimitiveModel(_gltfModel, primitive, transform, weights, isMirrored, _basePath, _texturePath, _materialSystem, _isStatic);
                LOG(Logger::INFO, "Mesh: ", mesh.name, ", Primitive vertices: ", model->Vertices.size());
                _models.push_back(std::move(model));
            }
        }

        for (int32 child : node.children)
        {
            if (child >= 0 && child < static_cast<int32>(_gltfModel.nodes.size()))
            {
                ProcessNode(_gltfModel, child, transform, _models, _isStatic, _basePath, _texturePath, _materialSystem);
            }
        }
    }

    void PrintMirroredNodes(const tinygltf::Model& model) {
        for (size_t i = 0; i < model.nodes.size(); ++i) {
            const auto& node = model.nodes[i];

            // Compute local transform (not including parent)
            glm::mat4 localMatrix(1.0f);

            if (node.matrix.size() == 16) {
                localMatrix = glm::make_mat4x4(node.matrix.data());
            }
            else {
                glm::vec3 translation = node.translation.size() == 3
                    ? glm::vec3(node.translation[0], node.translation[1], node.translation[2])
                    : glm::vec3(0.0f);

                glm::quat rotation = node.rotation.size() == 4
                    ? glm::quat(static_cast<float>(node.rotation[3]), static_cast<float>(node.rotation[0]), static_cast<float>(node.rotation[1]), static_cast<float>(node.rotation[2]))
                    : glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

                glm::vec3 scale = node.scale.size() == 3
                    ? glm::vec3(node.scale[0], node.scale[1], node.scale[2])
                    : glm::vec3(1.0f);

                localMatrix = glm::translate(glm::mat4(1.0f), translation)
                    * glm::mat4_cast(rotation)
                    * glm::scale(glm::mat4(1.0f), scale);
            }

            // Check if this matrix flips handedness (is mirrored)
            glm::vec3 x = glm::vec3(localMatrix[0]);
            glm::vec3 y = glm::vec3(localMatrix[1]);
            glm::vec3 z = glm::vec3(localMatrix[2]);

            bool isMirrored = glm::dot(glm::cross(x, y), z) < 0.0f;

            if (isMirrored) {
                std::string name = node.name.empty() ? "<unnamed>" : node.name;
                std::string meshInfo = (node.mesh >= 0 && node.mesh < (int)model.meshes.size())
                    ? model.meshes[node.mesh].name
                    : "<no mesh>";
                LOG(Logger::INFO, "Mirrored Node: ", name,
                    " (index ", i, "), Mesh: ", meshInfo);
            }
        }
    }

}

GltfLoader::GltfLoader(VesperApp& _app, Device& _device, MaterialSystem& _materialSystem)
    : m_app(_app), m_device{ _device }, m_materialSystem{ _materialSystem }
{
}

std::vector<std::unique_ptr<ModelData>> GltfLoader::LoadModel(const std::string& _fileName, bool _isStatic)
{
    const bool bIsFilepath = FileSystem::IsFilePath(_fileName);

    const std::string basePath = bIsFilepath ? FileSystem::GetDirectoryPath(_fileName) : m_app.GetConfig().ModelsPath;
    const std::string texturePath = bIsFilepath ? FileSystem::GetDirectoryPath(_fileName, true) + m_app.GetConfig().TexturesFolderName : m_app.GetConfig().TexturesPath;
    const std::string actualFilename = bIsFilepath ? FileSystem::GetFileName(_fileName) : _fileName;

    std::vector<std::unique_ptr<ModelData>> models;

    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret = false;
    if (actualFilename.rfind(".glb") != std::string::npos) 
    {
        ret = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, basePath + actualFilename);
    }
    else
    {
        ret = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, basePath + actualFilename);
    }

    if (!warn.empty())
    {
        LOG(Logger::WARNING, warn);
    }

    if (!ret) 
    {
        throw std::runtime_error(err);
    }

    std::vector<MorphAnimation> animations;
    LoadAnimations(gltfModel, animations);

    PrintMirroredNodes(gltfModel);

    int32 sceneIndex = gltfModel.defaultScene >= 0 ? gltfModel.defaultScene : 0;
    if (sceneIndex < static_cast<int32>(gltfModel.scenes.size())) {
        const tinygltf::Scene& scene = gltfModel.scenes[sceneIndex];
        for (int32 nodeIdx : scene.nodes) 
        {
            if (nodeIdx >= 0 && nodeIdx < static_cast<int32>(gltfModel.nodes.size())) 
            {
                ProcessNode(gltfModel, nodeIdx, glm::mat4(1.0f), models, _isStatic, basePath, texturePath, m_materialSystem);
            }
        }
    }

    for (auto& model : models)
    {
        model->Animations = animations;
    }

    LOG(Logger::INFO, "Total primitives processed: ", models.size());
    LOG_NL();
    LOG_NL();

    return models;
}

VESPERENGINE_NAMESPACE_END
