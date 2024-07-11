#include "Display.hpp"
#include "Renderer.hpp"
#include "Window.hpp"

int main() {
    Display display;
    Window window(display);
    Renderer renderer(window);

    while (!window.should_close()) {
        display.poll_events();
        renderer.render();
    }
}
