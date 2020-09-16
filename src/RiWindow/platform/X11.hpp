#pragma once
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GLFW/glfw3native.h>

namespace rise {
    namespace platform {
        struct X11Handle {
            ::Display *display;
            ::Window window;
        };

        inline X11Handle getHandle(GLFWwindow* window) {
            return { glfwGetX11Display(), glfwGetX11Window(window) };
        }
    }
}