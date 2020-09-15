#include <RiWindow/window.hpp>
#include <RiWindow/platform.hpp>

using namespace rise;

int main() {
    rise::Window gabeWindow("Ave Gaben!", Extent2D(800, 600), WindowEvent::FullScreen);

    std::unordered_set<rise::Window> windows;
    windows.insert(std::move(gabeWindow));
    windows.emplace("Ave Hackman!", Extent2D(800, 600));

    for (auto const &window : windows) {
        window.event().subscribe([&](WindowUserEvent event) {
            switch (event) {
                case WindowUserEvent::Minimize:
                    std::cout << "minimize" << std::endl;
                    break;
                case WindowUserEvent::Maximize:
                    std::cout << "maximize" << std::endl;
                    break;
                case WindowUserEvent::Focused:
                    std::cout << "focused" << std::endl;
                    break;
                case WindowUserEvent::Relaxed:
                    std::cout << "relaxed" << std::endl;
                    break;
                case WindowUserEvent::ShouldClose:
                    std::cout << "close" << std::endl;
                    windows.erase(window);
                    break;
            }

        });

        window.getNativeHandle();

        auto area = window.size() | transform([](Extent2D size) {
            return size.width * size.height;
        });

        area.subscribe([](unsigned v) {
            std::cout << "Current window area is: " << v << std::endl;
        });

        window.pos().subscribe([](Point2D p) {
            std::cout << "Current window position is: " << p.x << " " << p.y << std::endl;
        });
    }

    while (!windows.empty());
}