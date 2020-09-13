#include "window.hpp"
#include <iostream>
#include <atomic>

namespace rise {
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

            auto scheduler = observe_on_new_thread();
            windowCreator.get_observable().subscribe_on(scheduler).subscribe([this](Extent2D size) {
                handle = glfwCreateWindow(size.width, size.height, "undefined window", nullptr, nullptr);
                if(!handle) {
                    throw std::runtime_error("fail create window");
                }
            });

            auto glfwUpdate = observable<>::interval(interval, observe_on_new_thread());

            glfwUpdate.subscribe([](int) {
                glfwPollEvents();
            });
        }

        ~GlfwInitializer() {
            glfwTerminate();
        }

        GLFWwindow *createGlfwWindow(Extent2D size) {
            windowCreator.get_subscriber().on_next(size);
            return handle;
        }

        subject<Extent2D> windowCreator;
        std::atomic<GLFWwindow*> handle;
    };

    impl::WindowHandle createWindow(Extent2D size) {
        static GlfwInitializer initializer;

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
            }, [&fn](observable<T> const &v) {
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
        std::optional<WindowProperty<WindowEvent>> const &events
    ) : mWindow(createWindow(size)) {

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
}