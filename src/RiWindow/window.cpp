#define GLFW_INCLUDE_VULKAN
#include "window.hpp"
#include <iostream>
#include "platform.hpp"

namespace rise {
    WindowHandle Window::nativeHandle() const {
        return platform::getHandle(mWindow.get());
    }

    Window *getFromGlfw(GLFWwindow *window) {
        return reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    }

    struct GlfwInitializer {
        GlfwInitializer() {
            if (!glfwInit()) {
                throw std::runtime_error("Fail to initialize glfw!");
            }

            auto rate = glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate;
            auto interval = std::chrono::milliseconds(1000) / rate;

            auto glfwUpdate = observable<>::interval(interval, observe_on_new_thread());

            glfwUpdate.subscribe([this](int) {
                glfwPollEvents();

                if (needCreate) {
                    handle = glfwCreateWindow(width, height, "undefined window", nullptr, nullptr);
                    if (!handle) {
                        throw std::runtime_error("fail create window");
                    }
                    wait = false;
                    needCreate = false;
                }
            });
        }

        ~GlfwInitializer() {
            glfwTerminate();
        }

        GLFWwindow *createGlfwWindow(Extent2D size) {
            width = size.width;
            height = size.height;
            needCreate = true;
            while (wait);
            wait = true;
            return handle;
        }

        std::atomic<GLFWwindow *> handle;
        std::atomic<bool> wait = true;
        std::atomic<bool> needCreate = false;
        std::atomic<Width> width;
        std::atomic<Height> height;
    };


    impl::WindowHandle createWindow(Extent2D size, WindowClientApi api) {
        static GlfwInitializer initializer;

        switch (api) {
            case rise::WindowClientApi::NoApi:
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
                break;
            default:
                break;
        }

        return impl::WindowHandle {
            initializer.createGlfwWindow(size),
            glfwDestroyWindow
        };
    }

    template<typename T, typename FnT>
    void subscribeOnUser(WindowProperty<T> const &prop, FnT &&fn) {
        return std::visit(overloaded {
            [&fn](T const &v) {
                fn(v);
            }, [&fn](observable <T> const &v) {
                v.subscribe(fn);
            },
        }, prop);
    }

    template<typename T, typename FnT>
    void subscribeOnUser(std::optional<WindowProperty<T>> const &prop, FnT &&fn) {
        if (prop.has_value()) {
            subscribeOnUser(*prop, fn);
        }
    }

    Window::Window(
        WindowProperty<std::string_view> const &title,
        Extent2D const &size,
        std::optional<WindowProperty<WindowEvent>> const &events,
        WindowClientApi api
    ) : mWindow(createWindow(size, api)) {

        glfwSetWindowUserPointer(mWindow.get(), this);
        glfwSetWindowCloseCallback(mWindow.get(), &closeCallback);
        glfwSetWindowSizeCallback(mWindow.get(), &resizeCallback);
        glfwSetFramebufferSizeCallback(mWindow.get(), &frameBufferResizeCallback);
        glfwSetWindowPosCallback(mWindow.get(), &moveCallback);
        glfwSetWindowMaximizeCallback(mWindow.get(), &maximizeCallback);
        glfwSetWindowFocusCallback(mWindow.get(), &focusCallback);

        subscribeOnUser(title, [this](std::string_view title) {
            glfwSetWindowTitle(mWindow.get(), title.data());
        });

        subscribeOnUser(events, [this](WindowEvent event) {
            auto settings = glfwGetVideoMode(glfwGetPrimaryMonitor());

            switch (event) {
                case WindowEvent::FullScreen:
                    glfwSetWindowMonitor(mWindow.get(), glfwGetPrimaryMonitor(), 0, 0,
                        settings->width, settings->height, settings->refreshRate);
                    break;
                case WindowEvent::Windowed:
                    glfwSetWindowMonitor(mWindow.get(), glfwGetPrimaryMonitor(),
                        settings->width / 3, settings->height / 3,
                        settings->width / 3, settings->height / 3,
                        settings->refreshRate);
                    break;
                case WindowEvent::Borderless:
                    glfwSetWindowAttrib(mWindow.get(), GLFW_DECORATED, GLFW_FALSE);
                    break;
                case WindowEvent::Framed:
                    glfwSetWindowAttrib(mWindow.get(), GLFW_DECORATED, GLFW_TRUE);
                    break;
                default:
                    break;
            }
        });
    }

    Window::Window(Window &&rhs) noexcept:
        mEvents(std::move(rhs.mEvents)),
        mSize(std::move(rhs.mSize)),
        mPos(std::move(rhs.mPos)),
        mFrameBufferSize(std::move(rhs.mFrameBufferSize)),
        mWindow(std::move(rhs.mWindow)) {

        glfwSetWindowUserPointer(mWindow.get(), this);
    }

    Window &Window::operator=(Window &&rhs) noexcept {
        mEvents = std::move(rhs.mEvents);
        mSize = std::move(rhs.mSize);
        mPos = std::move(rhs.mPos);
        mFrameBufferSize = std::move(rhs.mFrameBufferSize);
        mWindow = std::move(rhs.mWindow);
        glfwSetWindowUserPointer(mWindow.get(), this);
        return *this;
    }

    void Window::closeCallback(GLFWwindow *glfWindow) {
        auto window = getFromGlfw(glfWindow);
        window->mEvents.get_subscriber().on_next(WindowUserEvent::ShouldClose);
    }

    void Window::resizeCallback(GLFWwindow *glfWindow, int width, int height) {
        auto window = getFromGlfw(glfWindow);
        window->mSize.get_subscriber().on_next(Extent2D(width, height));
    }

    void Window::frameBufferResizeCallback(GLFWwindow *glfWindow, int width, int height) {
        auto window = getFromGlfw(glfWindow);
        window->mFrameBufferSize.get_subscriber().on_next(Extent2D(width, height));
    }

    void Window::moveCallback(GLFWwindow *glfWindow, int x, int y) {
        auto window = getFromGlfw(glfWindow);
        window->mPos.get_subscriber().on_next(Point2D(x, y));
    }

    void Window::maximizeCallback(GLFWwindow *glfWindow, int val) {
        auto window = getFromGlfw(glfWindow);
        window->mEvents.get_subscriber().on_next(
            val ? WindowUserEvent::Maximize : WindowUserEvent::Minimize);
    }

    void Window::focusCallback(GLFWwindow *glfWindow, int val) {
        auto window = getFromGlfw(glfWindow);
        window->mEvents.get_subscriber().on_next(
            val ? WindowUserEvent::Focused : WindowUserEvent::Relaxed);
    }

    std::vector<std::string> windowVulkanExtensions() {
        uint32_t count;
        const char **extensions = glfwGetRequiredInstanceExtensions(&count);
        return std::vector<std::string>(extensions, extensions + count);
    }

    VkSurfaceKHR Window::vulkanSurface(VkInstance instance) const {
        VkSurfaceKHR surface;
        VkResult err = glfwCreateWindowSurface(instance, mWindow.get(), NULL, &surface);
        if (err) {
            throw std::runtime_error("Vulkan surface create error");
        }
        return surface;
    }
}