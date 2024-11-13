#pragma once

#include "vulkan/Renderer.hpp"
#include "wayland/Window.hpp"

class MainWindow : public Window {
public:
    explicit MainWindow(Display& display);

    virtual void key_down(xkb_keysym_t keysym, bool shift, bool ctrl, bool alt) noexcept override;
    virtual void key_up(xkb_keysym_t keysym, bool shift, bool ctrl, bool alt) noexcept override;
    virtual void key_modifiers(bool shift, bool ctrl, bool alt) noexcept override;
    virtual void pointer_click(uint32_t button, wl_pointer_button_state state) noexcept override;
    virtual void pointer_motion(float x, float y) noexcept override;
    virtual void reconfigure() noexcept override;
    virtual void text(const std::string& str) const noexcept override;

    void render();

private:
    Renderer _renderer;
};
