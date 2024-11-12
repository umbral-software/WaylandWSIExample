#include "vulkan/Renderer.hpp"
#include "wayland/Display.hpp"
#include "wayland/Window.hpp"

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>

int main() {
    IMGUI_CHECKVERSION();
    const auto imgui = ImGui::CreateContext();

    {
        Display display;
        Window window(display);
        Renderer renderer(window);

        while (!window.should_close()) {
            display.poll_events();
            window.update_ui();
            renderer.render();
        }
    }

    ImGui::DestroyContext(ImGui::GetCurrentContext());
}
