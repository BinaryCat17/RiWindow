#pragma once
#include <Windows.h>
#include <GLFW/glfw3native.h.h>

namespace rise {
    namespace platform {
        struct Win32Handle {
            HWND window;
        };

        inline Win32Handle getHandle(GLFWwindow* window) {
            return { glfwGetWin32Window(window) };
        }
    }
}