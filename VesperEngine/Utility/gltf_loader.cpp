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

#define __STDC_LIB_EXT1__					// for using sprintf_s
#define TINYGLTF_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ThirdParty/tiny_gltf.h"

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

    std::string GetTexturePath(const tinygltf::Model& _model, int _texIndex, const std::string& _modelPath, const std::string& _texturePath)
    {
        if (_texIndex < 0 || _texIndex >= static_cast<int>(_model.textures.size()))
        {
            return "";
        }

        const tinygltf::Texture& texture = _model.textures[_texIndex];
        if (texture.source < 0 || texture.source >= static_cast<int>(_model.images.size()))
        {
            return "";
        }

        const tinygltf::Image& image = _model.images[texture.source];
        if (!image.uri.empty() && image.uri.find("data:") != 0)
        {
            return _modelPath + image.uri;
        }

        // TODO: handle embedded image data using TextureSystem directly
        LOG(Logger::WARNING, "Embedded image not supported: ", image.name);
        return "";
    }
}

GltfLoader::GltfLoader(VesperApp& _app, Device& _device, MaterialSystem& _materialSystem)
    : m_app(_app)
    , m_device{ _device }
    , m_materialSystem{ _materialSystem }
{
}

std::vector<std::unique_ptr<ModelData>> GltfLoader::LoadModel(const std::string& _fileName, bool _isStatic)
{
    const bool bIsFilepath = FileSystem::IsFilePath(_fileName);

    const std::string modelPath = bIsFilepath ? FileSystem::GetDirectoryPath(_fileName) : m_app.GetConfig().ModelsPath;
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
        ret = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, modelPath + actualFilename);
    }
    else
    {
        ret = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, modelPath + actualFilename);
    }

    if (!warn.empty())
    {
        LOG(Logger::WARNING, warn);
    }

    if (!ret)
    {
        throw std::runtime_error(err);
    }

    for (const auto& mesh : gltfModel.meshes)
    {
        for (const auto& primitive : mesh.primitives)
        {
            auto modelData = std::make_unique<ModelData>();

            // Material
            if (primitive.material >= 0 && primitive.material < static_cast<int>(gltfModel.materials.size()))
            {
                const tinygltf::Material& gltfMaterial = gltfModel.materials[primitive.material];
                const tinygltf::PbrMetallicRoughness& pbr = gltfMaterial.pbrMetallicRoughness;

                std::string roughTex = GetTexturePath(gltfModel, pbr.metallicRoughnessTexture.index, texturePath, texturePath);
                std::string metallicTex = roughTex;
                std::string emissiveTex = GetTexturePath(gltfModel, gltfMaterial.emissiveTexture.index, texturePath, texturePath);
                std::string normalTex = GetTexturePath(gltfModel, gltfMaterial.normalTexture.index, texturePath, texturePath);

                modelData->Material = m_materialSystem.CreateMaterial(
                    gltfMaterial.name,
                    { roughTex, metallicTex, "", emissiveTex, normalTex },
                    { static_cast<float>(pbr.roughnessFactor), static_cast<float>(pbr.metallicFactor), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
                    gltfMaterial.alphaMode == "BLEND",
                    MaterialType::PBR);
            }
            else
            {
                modelData->Material = m_materialSystem.CreateMaterial(
                    "_DefaultGltfPBR_",
                    { "", "", "", "", "" },
                    { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
                    false,
                    MaterialType::PBR);
            }

            // Attributes
            std::vector<glm::vec3> positions;
            std::vector<glm::vec3> normals;
            std::vector<glm::vec2> uvs;

            auto itPos = primitive.attributes.find("POSITION");
            if (itPos != primitive.attributes.end())
            {
                ReadAccessor(gltfModel, gltfModel.accessors[itPos->second], positions);
            }

            auto itNorm = primitive.attributes.find("NORMAL");
            if (itNorm != primitive.attributes.end())
            {
                ReadAccessor(gltfModel, gltfModel.accessors[itNorm->second], normals);
            }

            auto itUV = primitive.attributes.find("TEXCOORD_0");
            if (itUV != primitive.attributes.end())
            {
                ReadAccessor(gltfModel, gltfModel.accessors[itUV->second], uvs);
                for (auto& uv : uvs)
                {
                    uv.y = 1.0f - uv.y;
                }
            }

            std::vector<uint32> indices;
            if (primitive.indices >= 0)
            {
                ReadAccessor(gltfModel, gltfModel.accessors[primitive.indices], indices);
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
                if (vIndex < positions.size()) vertex.Position = positions[vIndex];
                if (vIndex < normals.size()) vertex.Normal = normals[vIndex];
                if (vIndex < uvs.size()) vertex.UV = uvs[vIndex];
                vertex.Color = { 1.0f, 1.0f, 1.0f };

                modelData->Vertices[i] = vertex;
                modelData->Indices[i] = static_cast<uint32>(i);
            }

            modelData->IsStatic = _isStatic;

            LOG(Logger::INFO, "Mesh: ", mesh.name, ", Primitive vertices: ", modelData->Vertices.size());

            models.push_back(std::move(modelData));
        }
    }

    LOG(Logger::INFO, "Total primitives processed: ", models.size());
    LOG_NL();
    LOG_NL();

    return models;
}

VESPERENGINE_NAMESPACE_END
