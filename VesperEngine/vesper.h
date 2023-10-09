#pragma once


#define GLM_FORCE_INTRINSICS
//#define GLM_FORCE_SSE2		// or GLM_FORCE_SSE42 or else, but the above one use compiler to find out which one is enabled
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/ext.hpp>


#include "Core/core_defines.h"
#include "Core/memory_copy.h"

#include "Backend/model_data.h"
#include "Backend/pipeline.h"
#include "Backend/device.h"
#include "Backend/buffer.h"
#include "Backend/swap_chain.h"
#include "Backend/renderer.h"

#include "Components/graphics_components.h"
#include "Components/object_components.h"
#include "Components/camera_components.h"

#include "Systems/game_entity_system.h"
#include "Systems/model_system.h"
#include "Systems/core_render_system.h"
#include "Systems/simple_render_system.h"
#include "Systems/camera_system.h"

#include "Utility/hash.h"
#include "Utility/primitive_factory.h"
#include "Utility/obj_loader.h"

#include "App/config.h"
#include "App/window_handle.h"
#include "App/vesper_app.h"

#include "ECS/ecs.h"
