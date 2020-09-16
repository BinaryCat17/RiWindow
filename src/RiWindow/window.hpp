#pragma once
#include <rxcpp/rx.hpp>
#include <string_view>
#include <RiUtil.hpp>
#include <GLFW/glfw3.h>
#include <atomic>
#include <variant>
#include "platform/defines.hpp"
#include <GLFW/glfw3.h>

struct VkInstance_T;
typedef struct VkInstance_T *VkInstance;

struct VkSurfaceKHR_T;
typedef struct VkSurfaceKHR_T *VkSurfaceKHR;

namespace rise {
    using namespace rxcpp;
    using namespace rxcpp::operators;
    using namespace rxcpp::subjects;

    enum class WindowUserEvent {
        Minimize,
        Maximize,
        Focused,
        Relaxed,
        ShouldClose,
    };

    enum class WindowEvent {
        FullScreen,
        Windowed,
        Borderless,
        Framed,
    };

    enum class WindowClientApi {
        NoApi,
    };

    namespace impl {
        using WindowHandle = std::unique_ptr<GLFWwindow, decltype(glfwDestroyWindow) *>;
    }

    template<typename T>
    using WindowProperty = std::variant<observable<T>, T>;

    class Window : NonCopyable {
    public:
        explicit Window(
            WindowProperty<std::string_view> const &title,
            Extent2D const &size,
            std::optional<WindowProperty<WindowEvent>> const &events = {},
            WindowClientApi api = WindowClientApi::NoApi
        );

        Window(Window &&rhs) noexcept;

        Window &operator=(Window &&rhs) noexcept;

        [[nodiscard]] observable<WindowUserEvent> event() const {
            return mEvents.get_observable();
        }

        [[nodiscard]] observable<Extent2D> size() const {
            return mSize.get_observable();
        }

        [[nodiscard]] observable<Point2D> pos() const {
            return mPos.get_observable();
        }

        [[nodiscard]] observable<Extent2D> frameBufferSize() const {
            return mFrameBufferSize.get_observable();
        }

        friend bool operator==(const Window &lhs, const Window &rhs) {
            return lhs.mWindow == rhs.mWindow;
        }

        friend bool operator!=(const Window &lhs, const Window &rhs) {
            return !(rhs == lhs);
        }

        [[nodiscard]] WindowHandle nativeHandle() const;

        [[nodiscard]] std::vector<std::string> vulkanExtensions() const;

        [[nodiscard]] VkSurfaceKHR vulkanSurface(VkInstance instance) const;

    private:
        friend struct std::hash<Window>;

        subject<WindowUserEvent> mEvents;

        subject<Extent2D> mSize;

        subject<Point2D> mPos;

        subject<Extent2D> mFrameBufferSize;

        static void closeCallback(GLFWwindow *glfWindow);

        static void resizeCallback(GLFWwindow *glfWindow, int, int);

        static void frameBufferResizeCallback(GLFWwindow *glfWindow, int, int);

        static void moveCallback(GLFWwindow *glfWindow, int, int);

        static void maximizeCallback(GLFWwindow *glfWindow, int);

        static void focusCallback(GLFWwindow *glfWindow, int);

        impl::WindowHandle mWindow;
    };
}

namespace std {
    template<>
    struct hash<rise::Window> {
        std::size_t operator()(rise::Window const &w) const noexcept {
            return hasher(w.mWindow);
        }

        std::hash<rise::impl::WindowHandle> hasher;
    };
}
