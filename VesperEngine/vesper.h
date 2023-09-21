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

#include "Backend/pipeline.h"
#include "Backend/device.h"
#include "Backend/swap_chain.h"
#include "Backend/renderer.h"

#include "Geometry/model.h"

#include "Scene/game_object.h"

#include "App/window_handle.h"

#include "Frontend/simple_render_system.h"