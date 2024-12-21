#pragma once

#include "Core/core_defines.h"

#include "Backend/device.h"
#include "Backend/model_data.h"

#include <string>
#include <memory>


VESPERENGINE_NAMESPACE_BEGIN

class VESPERENGINE_API ObjLoader
{
public:
	ObjLoader(Device& _device);
	~ObjLoader() = default;

	ObjLoader(const ObjLoader&) = delete;
	ObjLoader& operator=(const ObjLoader&) = delete;

public:
	std::unique_ptr<ModelData> LoadModel(const std::string& _filePath, bool _isStatic = true);

private:
	Device& m_device;
};

VESPERENGINE_NAMESPACE_END
