#include <stdexcept>
#include "window.hpp"

namespace rise {
    // util ---------------------------------------------------------------------------------------

    struct GlfwInitializer {
        GlfwInitializer() {
            if (!glfwInit()) {
                throw std::runtime_error("Fail to initialize glfw!");
            }
        }

        ~GlfwInitializer() {
            glfwTerminate();
        }
    };

    Window *getFromGlfw(GLFWwindow *window) {
        return reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    }

    // Window -------------------------------------------------------------------------------------

    Window::Window(string const &title, Extent2D extent) : Window(
            WindowBuilder().title(title).size(extent).buildGlfwWindow()) {}

    void Window::setTitle(string const &title) {
        glfwSetWindowTitle(mWindow.get(), title.c_str());
    }

    void Window::setSize(Extent2D size) {
        glfwSetWindowSize(mWindow.get(),
                static_cast<int>(size.width.value),
                static_cast<int>(size.height.value));
    }

    Extent2D Window::getSize() const {
        int width = 0;
        int height = 0;

        glfwGetWindowSize(mWindow.get(), &width, &height);

        return Extent2D(
                rise::Width(static_cast<unsigned>(width)),
                rise::Height(static_cast<unsigned>(height)));
    }

    Extent2D Window::getFramebufferSize() const {
        int width = 0;
        int height = 0;

        glfwGetFramebufferSize(mWindow.get(), &width, &height);

        return Extent2D(
                rise::Width(static_cast<unsigned>(width)),
                rise::Height(static_cast<unsigned>(height)));
    }

    void Window::setPosition(Point2D size) {
        glfwSetWindowSize(mWindow.get(),
                static_cast<int>(size.x.value),
                static_cast<int>(size.y.value));
    }

    Point2D Window::getPosition() const {
        int width = 0;
        int height = 0;

        glfwGetWindowPos(mWindow.get(), &width, &height);

        return Point2D(
                rise::X(static_cast<unsigned>(width)),
                rise::Y(static_cast<unsigned>(height)));
    }

    Window::Window(UniqueGlfwWindow window) : mWindow(move(window)) {
        glfwSetWindowUserPointer(mWindow.get(), this);
        glfwSetWindowCloseCallback(mWindow.get(), &closeCallback);
        glfwSetWindowSizeCallback(mWindow.get(), &resizeCallback);
        glfwSetFramebufferSizeCallback(mWindow.get(), &framebufferResizeCallback);
        glfwSetWindowPosCallback(mWindow.get(), &moveCallback);
    }

    void Window::closeCallback(GLFWwindow *glfWindow) {
        auto window = getFromGlfw(glfWindow);
        window->mOnClose.emit();
    }

    void Window::resizeCallback(GLFWwindow *glfWindow, int, int) {
        auto window = getFromGlfw(glfWindow);
        window->mOnResize.emit();
    }

    void Window::framebufferResizeCallback(GLFWwindow *glfWindow, int, int) {
        auto window = getFromGlfw(glfWindow);
        window->mOnFramebufferResize.emit();
    }

    void Window::moveCallback(GLFWwindow *glfWindow, int, int) {
        auto window = getFromGlfw(glfWindow);
        window->mOnMove.emit();
    }

    void poolEvents() {
        glfwPollEvents();
    }

    // WindowBuilder ------------------------------------------------------------------------------

    Window WindowBuilder::build() const {
        return Window(buildGlfwWindow());
    }

    UniqueGlfwWindow WindowBuilder::buildGlfwWindow() const {
        static GlfwInitializer initializer = {};

        if(mOpenGlVersion) {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, static_cast<int>(mOpenGlVersion->major));
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, static_cast<int>(mOpenGlVersion->minor));

            if (*mOpenGlVersion < Version(3, 2)) {
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
            } else {
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            }
        } else {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

        glfwWindowHint(GLFW_RESIZABLE, mResizable);
        glfwWindowHint(GLFW_VISIBLE, mVisible);
        glfwWindowHint(GLFW_DECORATED, mDecorated);
        glfwWindowHint(GLFW_FOCUSED, mFocused);
        glfwWindowHint(GLFW_FLOATING, mFloating);
        glfwWindowHint(GLFW_MAXIMIZED, mMaximized);
        glfwWindowHint(GLFW_CENTER_CURSOR, mCenterCursor);
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, mFocusOnShow);
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, mScaleToMonitor);

        auto window = glfwCreateWindow(
                static_cast<int>(mExtent.width.value),
                static_cast<int>(mExtent.height.value),
                mTitle.c_str(),
                nullptr,
                nullptr);

        if (!window) {
            char const *error = nullptr;
            glfwGetError(&error);
            throw std::runtime_error(error);
        }

        return UniqueGlfwWindow(window, &glfwDestroyWindow);
    }

}