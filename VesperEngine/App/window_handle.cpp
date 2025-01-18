// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\App\window_handle.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "App/window_handle.h"


VESPERENGINE_NAMESPACE_BEGIN

WindowHandle::WindowHandle(int32 _width, int32 _height, const std::string& _name)
	: m_width{_width}
	, m_height{ _height }
	, m_name{ _name }
{
}

WindowHandle::~WindowHandle()
{
}

VESPERENGINE_NAMESPACE_END