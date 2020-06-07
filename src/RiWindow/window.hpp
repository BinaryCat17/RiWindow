#pragma once

#include <RiUtil/types.hpp>
#include <signals.h>
#include <GLFW/glfw3.h>
#include <optional>
#include <string>
#include <memory>

namespace rise {
    using std::optional; using std::string; using std::unique_ptr;

    // util ---------------------------------------------------------------------------------------

    using UniqueGlfwWindow = unique_ptr<GLFWwindow, decltype(glfwDestroyWindow) *>;

    // Window -------------------------------------------------------------------------------------

    class Window {
        friend class WindowBuilder;
    public:
        explicit Window(string const &title, Extent2D extent);

        // property -------------------------------------------------------------------------------

        void setTitle(string const& title);

        void setSize(Extent2D size);

        Extent2D getSize() const;

        void setPosition(Point2D size);

        Point2D getPosition() const;

        Extent2D getFramebufferSize() const;

        // signals --------------------------------------------------------------------------------

        template<typename Fn>
        unsigned onClose(Fn slot) {
            return mOnClose.connect(std::move(slot));
        }

        bool onCloseDisconnect(unsigned id) {
            return mOnClose.disconnect(id);
        }

        template<typename Fn>
        unsigned onResize(Fn slot) {
            return mOnResize.connect(std::move(slot));
        }

        bool onResizeDisconnect(unsigned id) {
            return mOnResize.disconnect(id);
        }

        template<typename Fn>
        unsigned onFramebufferResize(Fn slot) {
            return mOnFramebufferResize.connect(std::move(slot));
        }

        bool onFramebufferResizeDisconnect(unsigned id) {
            return mOnFramebufferResize.disconnect(id);
        }

        template<typename Fn>
        unsigned onMove(Fn slot) {
            return mOnMove.connect(std::move(slot));
        }

        bool onMoveDisconnect(unsigned id) {
            return mOnMove.disconnect(id);
        }

    private:
        explicit Window(UniqueGlfwWindow window);

        static void closeCallback(GLFWwindow *glfWindow);

        static void resizeCallback(GLFWwindow *glfWindow, int, int);

        static void framebufferResizeCallback(GLFWwindow *glfWindow, int, int);

        static void moveCallback(GLFWwindow *glfWindow, int, int);

        UniqueGlfwWindow mWindow;
        vdk::signal<void()> mOnClose;
        vdk::signal<void()> mOnResize;
        vdk::signal<void()> mOnFramebufferResize;
        vdk::signal<void()> mOnMove;
    };

    void poolEvents();

    // WindowBuilder ------------------------------------------------------------------------------

    class WindowBuilder {
        friend class Window;
    public:
        Window build() const;

        WindowBuilder& openGL(Version version) {
            mOpenGlVersion = version;
            return *this;
        }

        WindowBuilder &size(Extent2D extent) {
            mExtent = extent;
            return *this;
        }

        WindowBuilder &title(string title) {
            mTitle = move(title);
            return *this;
        }

        WindowBuilder &resizable(bool value = true) {
            mResizable = value;
            return *this;
        }

        WindowBuilder &visible(bool value = true) {
            mVisible = value;
            return *this;
        }

        WindowBuilder &decorated(bool value = true) {
            mDecorated = value;
            return *this;
        }

        WindowBuilder &focused(bool value = true) {
            mFocused = value;
            return *this;
        }

        WindowBuilder &floating(bool value = true) {
            mFloating = value;
            return *this;
        }

        WindowBuilder &maximized(bool value = true) {
            mMaximized = value;
            return *this;
        }

        WindowBuilder &centerCursor(bool value = true) {
            mCenterCursor = value;
            return *this;
        }

        WindowBuilder &focusOnShow(bool value = true) {
            mFocusOnShow = value;
            return *this;
        }

        WindowBuilder &scaleToMonitor(bool value = true) {
            mScaleToMonitor = value;
            return *this;
        }

    private:
        UniqueGlfwWindow buildGlfwWindow() const;

        optional<Version> mOpenGlVersion;
        Extent2D mExtent = Extent2D{Width(800), Height(600)};
        string mTitle = "RiWindow";
        bool mResizable = true;
        bool mVisible = true;
        bool mDecorated = true;
        bool mFocused = false;
        bool mFloating = false;
        bool mMaximized = false;
        bool mCenterCursor = false;
        bool mFocusOnShow = false;
        bool mScaleToMonitor = false;
    };

}