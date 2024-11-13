#include "MainWindow.hpp"
#include "wayland/Display.hpp"

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>

int main() {
    IMGUI_CHECKVERSION();
    const auto imgui = ImGui::CreateContext();

    {
        Display display;
        MainWindow window(display);

        while (!window.should_close()) {
            display.poll_events();
            window.render();
        }
    }

    ImGui::DestroyContext(ImGui::GetCurrentContext());
}
