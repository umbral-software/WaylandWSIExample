#include "Display.hpp"
#include "Window.hpp"

int main() {
    Display display;
    Window window(display);

    while (!window.should_close()) {
        display.poll_events();
        window.render();
    }
}
