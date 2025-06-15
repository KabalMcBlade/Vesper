// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\vesper.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"
#include "Core/memory_copy.h"
#include "Core/glm_config.h"

#include "Backend/model_data.h"
#include "Backend/pipeline.h"
#include "Backend/device.h"
#include "Backend/buffer.h"
#include "Backend/swap_chain.h"
#include "Backend/offscreen_swap_chain.h"
#include "Backend/descriptors.h"
#include "Backend/renderer.h"
#include "Backend/offscreen_renderer.h"
#include "Backend/frame_info.h"

#include "Components/graphics_components.h"
#include "Components/object_components.h"
#include "Components/camera_components.h"
#include "Components/pipeline_components.h"

#include "Systems/game_entity_system.h"
#include "Systems/entity_handler_system.h"
#include "Systems/model_system.h"
#include "Systems/texture_system.h"
#include "Systems/material_system.h"
#include "Systems/base_render_system.h"
#include "Systems/master_render_system.h"
#include "Systems/phong_opaque_render_system.h"
#include "Systems/phong_transparent_render_system.h"
#include "Systems/pbr_opaque_render_system.h"
#include "Systems/pbr_transparent_render_system.h"
#include "Systems/skybox_render_system.h"
#include "Systems/camera_system.h"
#include "Systems/brdf_lut_generation_system.h"

#include "Utility/hash.h"
#include "Utility/logger.h"
#include "Utility/primitive_factory.h"
#include "Utility/obj_loader.h"
#include "Utility/gltf_loader.h"

#include "App/config.h"
#include "App/file_system.h"
#include "App/window_handle.h"
#include "App/vesper_app.h"

#include "ECS/ECS/ecs.h"
