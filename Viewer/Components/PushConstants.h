// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\Viewer\PushConstants.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once


struct ColorTintPushConstantData
{
    glm::vec3 ColorTint{ 1.0f };
    //uint8 padding[116]; // 128 (VESPERENGINE_PUSHCONSTANT_DEFAULTRANGE) - 12 (size of glm::vec3)
};
