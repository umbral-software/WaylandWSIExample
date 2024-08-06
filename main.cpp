#include "vulkan/Renderer.hpp"
#include "wayland/Display.hpp"
#include "wayland/Window.hpp"

int main() {
    Display display;
    Window window(display);
    Renderer renderer(window);

    while (!window.should_close()) {
        display.poll_events();
        renderer.render();
    }
}
