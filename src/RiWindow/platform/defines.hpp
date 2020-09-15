#pragma once

#if defined _WIN32
#   define RISE_OS_WIN32
#elif defined __linux__
#   define RISE_OS_LINUX
#endif

namespace rise {
    namespace platform {
        struct X11Handle;
        struct Win32Handle;
    }


#if defined(RISE_OS_WIN32)
    using WindowHandle = platform::Win32Handle;
#elif defined(RISE_OS_LINUX)
    using WindowHandle = platform::X11Handle;
#endif
}

