#include "MainWindow.hpp"
#include "wayland/Display.hpp"

#include <imgui.h>

static void real_main() {
    Display display;
    MainWindow window(display);

    while (!window.should_close()) {
        display.poll_events();
        window.render();
    }
}

int main() {
    IMGUI_CHECKVERSION();
    const auto imgui = ImGui::CreateContext();

    real_main();

    ImGui::DestroyContext(imgui);
}
