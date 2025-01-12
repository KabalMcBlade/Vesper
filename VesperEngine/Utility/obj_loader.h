// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Utility\obj_loader.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"

#include "Backend/device.h"
#include "Backend/model_data.h"

#include "Systems/material_system.h"

#include <string>
#include <memory>
#include <vector>


VESPERENGINE_NAMESPACE_BEGIN

class VesperApp;

class VESPERENGINE_API ObjLoader
{
public:
	ObjLoader(VesperApp& _app, Device& _device, MaterialSystem& _materialSystem);
	~ObjLoader() = default;

	ObjLoader(const ObjLoader&) = delete;
	ObjLoader& operator=(const ObjLoader&) = delete;

public:
	std::vector<std::unique_ptr<ModelData>> LoadModel(const std::string& _fileName, bool _isStatic = true);

private:
	VesperApp& m_app;
	Device& m_device;
	MaterialSystem& m_materialSystem;
};

VESPERENGINE_NAMESPACE_END
