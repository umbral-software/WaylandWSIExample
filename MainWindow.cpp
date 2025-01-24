#include "MainWindow.hpp"

#include <imgui.h>

MainWindow::MainWindow(Display& display)
    :Window(display)
    ,_renderer(*this)
{}

void MainWindow::render() {
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();
    ImGui::Render();

    _renderer.render();
}

void MainWindow::keyboard_events(const std::vector<std::unique_ptr<KeyboardEventBase>>& events) noexcept {

}

void MainWindow::pointer_events(const std::vector<std::unique_ptr<PointerEventBase>>& events) noexcept {

}

void MainWindow::touch_events(int id, const std::vector<std::unique_ptr<TouchEventBase>>& events) noexcept {

}
