#pragma once

#if defined(RISE_OS_WIN32)
#   define GLFW_EXPOSE_NATIVE_WIN32
#   include "platform/Win32.hpp"
#elif defined(RISE_OS_LINUX)
#   define GLFW_EXPOSE_NATIVE_X11
#   include "platform/X11.hpp"
#endif
