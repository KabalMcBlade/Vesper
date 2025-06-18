// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Utility\gltf_loader.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "Utility/gltf_loader.h"
#include <functional>
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

#define __STDC_LIB_EXT1__					// for using sprintf_s
#define TINYGLTF_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ThirdParty/include/tiny_gltf.h"

#include <stdexcept>
#include <unordered_map>

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
            else if constexpr (std::is_same_v<T, glm::vec2>)
            {
                const float* v = reinterpret_cast<const float*>(src);
                _out[i] = glm::vec2(v[0], v[1]);
            }
            else if constexpr (std::is_same_v<T, uint32>)
            {
                if (_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
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

    std::shared_ptr<TextureData> GetTexture(const tinygltf::Model& _model, int _texIndex, const std::string& _modelPath, MaterialSystem& _materialSystem)
    {
        if (_texIndex < 0 || _texIndex >= static_cast<int>(_model.textures.size()))
        {
            return nullptr;
        }

        const tinygltf::Texture& texture = _model.textures[_texIndex];
        if (texture.source < 0 || texture.source >= static_cast<int>(_model.images.size()))
        {
            return nullptr;
        }

        const tinygltf::Image& image = _model.images[texture.source];

        TextureSystem& textureSystem = _materialSystem.GetTextureSystem();

        if (!image.uri.empty() && image.uri.find("data:") != 0) 
        {
            return textureSystem.LoadTexture(_modelPath + image.uri);
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

    std::unique_ptr<ModelData> CreateModelData(const tinygltf::Model& _gltfModel,
                                               const tinygltf::Primitive& _primitive,
                                               const std::string& _basePath,
                                               MaterialSystem& _materialSystem,
                                               bool _isStatic)
    {
        auto modelData = std::make_unique<ModelData>();

        if (_primitive.material >= 0 &&
            _primitive.material < static_cast<int>(_gltfModel.materials.size()))
        {
            const tinygltf::Material& gltfMaterial = _gltfModel.materials[_primitive.material];
            const tinygltf::PbrMetallicRoughness& pbr = gltfMaterial.pbrMetallicRoughness;

            auto roughTex = GetTexture(_gltfModel, pbr.metallicRoughnessTexture.index, _basePath, _materialSystem);
            auto metallicTex = roughTex;
            auto albedoTex = GetTexture(_gltfModel, pbr.baseColorTexture.index, _basePath, _materialSystem);
            auto occlusionTex = GetTexture(_gltfModel, gltfMaterial.occlusionTexture.index, _basePath, _materialSystem);
            auto emissiveTex = GetTexture(_gltfModel, gltfMaterial.emissiveTexture.index, _basePath, _materialSystem);
            auto normalTex = GetTexture(_gltfModel, gltfMaterial.normalTexture.index, _basePath, _materialSystem);

            modelData->Material = _materialSystem.CreateMaterial(
                gltfMaterial.name,
                { roughTex, metallicTex, nullptr, emissiveTex, normalTex, albedoTex, occlusionTex },
                { static_cast<float>(pbr.roughnessFactor),
                 static_cast<float>(pbr.metallicFactor), 0.0f, 0.0f, 0.0f, 0.0f,
                 0.0f },
                gltfMaterial.alphaMode == "BLEND", MaterialType::PBR);
        }
        else
        {
            std::vector<std::shared_ptr<vesper::TextureData>> emptyTextures = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
            std::vector<std::any> emptyMetadata = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

            modelData->Material = _materialSystem.CreateMaterial(
                "_DefaultGltfPBR_", emptyTextures, emptyMetadata, false,
                MaterialType::PBR);
        }

        // Attributes
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;
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

        auto itUV = _primitive.attributes.find("TEXCOORD_0");
        if (itUV != _primitive.attributes.end())
        {
            ReadAccessor(_gltfModel, _gltfModel.accessors[itUV->second], uvs);
        }

        auto itColor = _primitive.attributes.find("COLOR_0");
        if (itColor != _primitive.attributes.end())
        {
            ReadColorAccessor(_gltfModel, _gltfModel.accessors[itColor->second], colors);
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

        modelData->Vertices.resize(indices.size());
        modelData->Indices.resize(indices.size());

        for (size_t i = 0; i < indices.size(); ++i)
        {
            uint32 vIndex = indices[i];
            Vertex vertex{};
            if (vIndex < positions.size())
            {
                vertex.Position = positions[vIndex];
            }
            if (vIndex < normals.size())
            {
                vertex.Normal = normals[vIndex];
            }
            if (vIndex < uvs.size())
            {
                vertex.UV = uvs[vIndex];
            }
            if (vIndex < colors.size())
            {
                vertex.Color = colors[vIndex];
            }
            else
            {
                vertex.Color = { 1.0f, 1.0f, 1.0f };
            }

            modelData->Vertices[i] = vertex;
            modelData->Indices[i] = static_cast<uint32>(i);
        }

        modelData->IsStatic = _isStatic;
        return modelData;
    }

    inline void DecomposeMatrixTRS(const glm::mat4& _mat, glm::vec3& _trans, glm::quat& _rot, glm::vec3& _scale)
    {
        _trans = glm::vec3(_mat[3]);
        glm::vec3 col0 = glm::vec3(_mat[0]);
        glm::vec3 col1 = glm::vec3(_mat[1]);
        glm::vec3 col2 = glm::vec3(_mat[2]);

        _scale.x = glm::length(col0);
        _scale.y = glm::length(col1);
        _scale.z = glm::length(col2);

        if (_scale.x != 0.0f) col0 /= _scale.x;
        if (_scale.y != 0.0f) col1 /= _scale.y;
        if (_scale.z != 0.0f) col2 /= _scale.z;

        glm::mat3 rotMat;
        rotMat[0] = col0;
        rotMat[1] = col1;
        rotMat[2] = col2;
        _rot = glm::quat_cast(rotMat);
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

    std::function<void(int, int)> processNode = [&](int nodeIndex, int parent)
    {
        const tinygltf::Node& node = gltfModel.nodes[nodeIndex];

        glm::vec3 translation{0.0f};
        glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
        glm::vec3 scale{1.0f};

        if (!node.matrix.empty())
        {
            glm::mat4 mat = glm::make_mat4(node.matrix.data());
            DecomposeMatrixTRS(mat, translation, rotation, scale);
        }
        else
        {
            if (node.translation.size() == 3)
            {
                translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
            }
            if (node.rotation.size() == 4)
            {
                rotation = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
            }
            if (node.scale.size() == 3)
            {
                scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
            }
        }

        int currentParent = parent;

        if (node.mesh >= 0 && node.mesh < static_cast<int>(gltfModel.meshes.size()))
        {
            const tinygltf::Mesh& mesh = gltfModel.meshes[node.mesh];
            bool first = true;
            for (const auto& prim : mesh.primitives)
            {
                auto model = CreateModelData(gltfModel, prim, basePath, m_materialSystem, _isStatic);
                model->Parent = parent;
                model->Translation = translation;
                model->Rotation = rotation;
                model->Scale = scale;
                models.push_back(std::move(model));
                if (first)
                {
                    currentParent = static_cast<int>(models.size()) - 1;
                    first = false;
                }
            }
        }
        else
        {
            auto model = std::make_unique<ModelData>();
            model->Material = m_materialSystem.CreateMaterial(MaterialSystem::DefaultPhongMaterial);
            model->IsStatic = _isStatic;
            model->Parent = parent;
            model->Translation = translation;
            model->Rotation = rotation;
            model->Scale = scale;
            models.push_back(std::move(model));
            currentParent = static_cast<int>(models.size()) - 1;
        }

        for (int child : node.children)
        {
            processNode(child, currentParent);
        }
    };

    const tinygltf::Scene& scene = gltfModel.scenes.empty() ? gltfModel.scenes.emplace_back() : gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];
    for (int nodeIndex : scene.nodes)
    {
        processNode(nodeIndex, -1);
    }

    LOG(Logger::INFO, "Total nodes processed: ", models.size());
    LOG_NL();
    LOG_NL();

    return models;
}

VESPERENGINE_NAMESPACE_END
