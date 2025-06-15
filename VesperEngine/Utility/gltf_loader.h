// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Utility\gltf_loader.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"

#include <string>
#include <memory>
#include <vector>

VESPERENGINE_NAMESPACE_BEGIN

class VesperApp;
class Device;
class MaterialSystem;

struct ModelData;

class VESPERENGINE_API GltfLoader
{
public:
    GltfLoader(VesperApp& _app, Device& _device, MaterialSystem& _materialSystem);
    ~GltfLoader() = default;

    GltfLoader(const GltfLoader&) = delete;
    GltfLoader& operator=(const GltfLoader&) = delete;

public:
    std::vector<std::unique_ptr<ModelData>> LoadModel(const std::string& _fileName, bool _isStatic = true);

private:
    VesperApp& m_app;
    Device& m_device;
    MaterialSystem& m_materialSystem;
};

VESPERENGINE_NAMESPACE_END
