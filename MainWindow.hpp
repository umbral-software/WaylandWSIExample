#pragma once

#include "vulkan/Renderer.hpp"
#include "wayland/Window.hpp"

class MainWindow final: public Window {
public:
    explicit MainWindow(Display& display);

    void keyboard_events(const std::vector<std::unique_ptr<KeyboardEventBase>>& events) noexcept final;
    void pointer_events(const std::vector<std::unique_ptr<PointerEventBase>>& events) noexcept final;
    void touch_events(int id, const std::vector<std::unique_ptr<TouchEventBase>>& events) noexcept final;

    void render();

private:
    Renderer _renderer;
};
