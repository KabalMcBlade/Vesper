#pragma once

#include "Core/core_defines.h"

#include "Backend/device.h"
#include "Backend/model_data.h"

#include <string>
#include <memory>
#include <vector>


VESPERENGINE_NAMESPACE_BEGIN

class VesperApp;

class VESPERENGINE_API ObjLoader
{
public:
	ObjLoader(VesperApp& _app, Device& _device);
	~ObjLoader() = default;

	ObjLoader(const ObjLoader&) = delete;
	ObjLoader& operator=(const ObjLoader&) = delete;

public:
	std::vector<std::unique_ptr<ModelData>> LoadModel(const std::string& _fileName, bool _isStatic = true);

private:
	VesperApp& m_app;
	Device& m_device;
};

VESPERENGINE_NAMESPACE_END
