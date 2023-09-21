#include "pch.h"

#include "App/window_handle.h"


VESPERENGINE_NAMESPACE_BEGIN

WindowHandle::WindowHandle(int32 _width, int32 _height, std::string _name)
	: m_width{_width}
	, m_height{ _height }
	, m_name{ _name }
{

}

WindowHandle::~WindowHandle()
{

}

VESPERENGINE_NAMESPACE_END