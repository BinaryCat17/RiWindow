#pragma once
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GLFW/glfw3native.h>

namespace rise {
    namespace platform {
        struct X11Handle {
            ::Display *display;
            ::Window window;
            ::XVisualInfo visual;
        };

        inline X11Handle getHandle(GLFWwindow* window) {
            auto display = glfwGetX11Display();
            XVisualInfo info;
            XMatchVisualInfo(display, XDefaultScreen(display), 24, TrueColor, &info);
            return { display, glfwGetX11Window(window), info };
        }
    }
}