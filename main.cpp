#include "imgui.h"
#include "vulkan/Renderer.hpp"
#include "wayland/Display.hpp"
#include "wayland/Window.hpp"

int main() {
    IMGUI_CHECKVERSION();
    const auto imgui = ImGui::CreateContext();

    {
        Display display;
        Window window(display);
        Renderer renderer(window);

        while (!window.should_close()) {
            display.poll_events();

            // FIXME: Don't resize every frame
            const auto buffer_size =    window.buffer_size();
            ImGui::GetIO().DisplaySize = { static_cast<float>(buffer_size.first), static_cast<float>(buffer_size.second) };
            renderer.resize(buffer_size); 

            ImGui::NewFrame();
            ImGui::ShowDemoWindow();
            ImGui::Render();
            renderer.render();

        }
    }

    ImGui::DestroyContext(imgui);
}
