#include "Window.hpp"

#include "Display.hpp"

#include <imgui.h>
#include <linux/input-event-codes.h>
#include <xkbcommon/xkbcommon-keysyms.h>

#include <cstring>

static constexpr uint32_t DEFAULT_HEIGHT = 600;
static constexpr uint32_t DEFAULT_WIDTH = 800;
static constexpr char WINDOW_TITLE[] = "Wayland Example";

static constexpr ImGuiKey xkb_to_imgui_key(xkb_keysym_t keysym) {
    switch (keysym) {
    case XKB_KEY_Tab:
        return ImGuiKey_Tab;
    case XKB_KEY_Left:
        return ImGuiKey_LeftArrow;
    case XKB_KEY_Right:
        return ImGuiKey_RightArrow;
    case XKB_KEY_Up:
        return ImGuiKey_UpArrow;
    case XKB_KEY_Down:
        return ImGuiKey_DownArrow;
    case XKB_KEY_Page_Up:
        return ImGuiKey_PageUp;
    case XKB_KEY_Page_Down:
        return ImGuiKey_PageDown;
    case XKB_KEY_Home:
        return ImGuiKey_Home;
    case XKB_KEY_End:
        return ImGuiKey_End;
    case XKB_KEY_Insert:
        return ImGuiKey_Insert;
    case XKB_KEY_Delete:
        return ImGuiKey_Delete;
    case XKB_KEY_BackSpace:
        return ImGuiKey_Backspace;
    case XKB_KEY_space:
        return ImGuiKey_Space;
    case XKB_KEY_Return:
        return ImGuiKey_Enter;
    case XKB_KEY_Escape:
        return ImGuiKey_Escape;
    case XKB_KEY_Control_L:
        return ImGuiKey_LeftCtrl;
    case XKB_KEY_Shift_L:
        return ImGuiKey_LeftShift;
    case XKB_KEY_Alt_L:
        return ImGuiKey_LeftAlt;
    case XKB_KEY_Super_L:
        return ImGuiKey_LeftSuper;
    case XKB_KEY_Control_R:
        return ImGuiKey_RightCtrl;
    case XKB_KEY_Shift_R:
        return ImGuiKey_RightShift;
    case XKB_KEY_Alt_R:
        return ImGuiKey_RightAlt;
    case XKB_KEY_Super_R:
        return ImGuiKey_RightSuper;
    case XKB_KEY_Menu:
        return ImGuiKey_Menu;
    case XKB_KEY_0:
        return ImGuiKey_0;
    case XKB_KEY_1:
        return ImGuiKey_1;
    case XKB_KEY_2:
        return ImGuiKey_2;
    case XKB_KEY_3:
        return ImGuiKey_3;
    case XKB_KEY_4:
        return ImGuiKey_4;
    case XKB_KEY_5:
        return ImGuiKey_5;
    case XKB_KEY_6:
        return ImGuiKey_6;
    case XKB_KEY_7:
        return ImGuiKey_7;
    case XKB_KEY_8:
        return ImGuiKey_8;
    case XKB_KEY_9:
        return ImGuiKey_9;
    case XKB_KEY_a:
    case XKB_KEY_A:
        return ImGuiKey_A;
    case XKB_KEY_b:
    case XKB_KEY_B:
        return ImGuiKey_B;
    case XKB_KEY_c:
    case XKB_KEY_C:
        return ImGuiKey_C;
    case XKB_KEY_d:
    case XKB_KEY_D:
        return ImGuiKey_D;
    case XKB_KEY_e:
    case XKB_KEY_E:
        return ImGuiKey_E;
    case XKB_KEY_f:
    case XKB_KEY_F:
        return ImGuiKey_F;
    case XKB_KEY_g:
    case XKB_KEY_G:
        return ImGuiKey_G;
    case XKB_KEY_h:
    case XKB_KEY_H:
        return ImGuiKey_H;
    case XKB_KEY_i:
    case XKB_KEY_I:
        return ImGuiKey_I;
    case XKB_KEY_j:
    case XKB_KEY_J:
        return ImGuiKey_J;
    case XKB_KEY_k:
    case XKB_KEY_K:
        return ImGuiKey_K;
    case XKB_KEY_l:
    case XKB_KEY_L:
        return ImGuiKey_L;
    case XKB_KEY_m:
    case XKB_KEY_M:
        return ImGuiKey_M;
    case XKB_KEY_n:
    case XKB_KEY_N:
        return ImGuiKey_N;
    case XKB_KEY_o:
    case XKB_KEY_O:
        return ImGuiKey_O;
    case XKB_KEY_p:
    case XKB_KEY_P:
        return ImGuiKey_P;
    case XKB_KEY_q:
    case XKB_KEY_Q:
        return ImGuiKey_Q;
    case XKB_KEY_r:
    case XKB_KEY_R:
        return ImGuiKey_R;
    case XKB_KEY_s:
    case XKB_KEY_S:
        return ImGuiKey_S;
    case XKB_KEY_t:
    case XKB_KEY_T:
        return ImGuiKey_T;
    case XKB_KEY_u:
    case XKB_KEY_U:
        return ImGuiKey_U;
    case XKB_KEY_v:
    case XKB_KEY_V:
        return ImGuiKey_V;
    case XKB_KEY_w:
    case XKB_KEY_W:
        return ImGuiKey_W;
    case XKB_KEY_x:
    case XKB_KEY_X:
        return ImGuiKey_X;
    case XKB_KEY_y:
    case XKB_KEY_Y:
        return ImGuiKey_Y;
    case XKB_KEY_z:
    case XKB_KEY_Z:
        return ImGuiKey_Z;
    case XKB_KEY_F1:
        return ImGuiKey_F1;
    case XKB_KEY_F2:
        return ImGuiKey_F2;
    case XKB_KEY_F3:
        return ImGuiKey_F3;
    case XKB_KEY_F4:
        return ImGuiKey_F4;
    case XKB_KEY_F5:
        return ImGuiKey_F5;
    case XKB_KEY_F6:
        return ImGuiKey_F6;
    case XKB_KEY_F7:
        return ImGuiKey_F7;
    case XKB_KEY_F8:
        return ImGuiKey_F8;
    case XKB_KEY_F9:
        return ImGuiKey_F9;
    case XKB_KEY_F10:
        return ImGuiKey_F10;
    case XKB_KEY_F11:
        return ImGuiKey_F11;
    case XKB_KEY_F12:
        return ImGuiKey_F12;
    case XKB_KEY_F13:
        return ImGuiKey_F13;
    case XKB_KEY_F14:
        return ImGuiKey_F14;
    case XKB_KEY_F15:
        return ImGuiKey_F15;
    case XKB_KEY_F16:
        return ImGuiKey_F16;
    case XKB_KEY_F17:
        return ImGuiKey_F17;
    case XKB_KEY_F18:
        return ImGuiKey_F18;
    case XKB_KEY_F19:
        return ImGuiKey_F19;
    case XKB_KEY_F20:
        return ImGuiKey_F20;
    case XKB_KEY_F21:
        return ImGuiKey_F21;
    case XKB_KEY_F22:
        return ImGuiKey_F22;
    case XKB_KEY_F23:
        return ImGuiKey_F23;
    case XKB_KEY_F24:
        return ImGuiKey_F24;
    case XKB_KEY_apostrophe:
        return ImGuiKey_Apostrophe;
    case XKB_KEY_comma:
        return ImGuiKey_Comma;
    case XKB_KEY_minus:
        return ImGuiKey_Minus;
    case XKB_KEY_period:
        return ImGuiKey_Period;
    case XKB_KEY_slash:
        return ImGuiKey_Slash;
    case XKB_KEY_semicolon:
        return ImGuiKey_Semicolon;
    case XKB_KEY_equal:
        return ImGuiKey_Equal;
    case XKB_KEY_braceleft:
        return ImGuiKey_LeftBracket;
    case XKB_KEY_backslash:
        return ImGuiKey_Backslash;
    case XKB_KEY_braceright:
        return ImGuiKey_RightBracket;
    case XKB_KEY_grave:
        return ImGuiKey_GraveAccent;
    case XKB_KEY_Caps_Lock:
        return ImGuiKey_CapsLock;
    case XKB_KEY_Scroll_Lock:
        return ImGuiKey_ScrollLock;
    case XKB_KEY_Num_Lock:
        return ImGuiKey_NumLock;
    case XKB_KEY_3270_PrintScreen:
        return ImGuiKey_PrintScreen;
    case XKB_KEY_Pause:
        return ImGuiKey_Pause;
    case XKB_KEY_KP_0:
        return ImGuiKey_Keypad0;
    case XKB_KEY_KP_1:
        return ImGuiKey_Keypad1;
    case XKB_KEY_KP_2:
        return ImGuiKey_Keypad2;
    case XKB_KEY_KP_3:
        return ImGuiKey_Keypad3;
    case XKB_KEY_KP_4:
        return ImGuiKey_Keypad4;
    case XKB_KEY_KP_5:
        return ImGuiKey_Keypad5;
    case XKB_KEY_KP_6:
        return ImGuiKey_Keypad6;
    case XKB_KEY_KP_7:
        return ImGuiKey_Keypad7;
    case XKB_KEY_KP_8:
        return ImGuiKey_Keypad8;
    case XKB_KEY_KP_9:
        return ImGuiKey_Keypad9;
    case XKB_KEY_KP_Decimal:
        return ImGuiKey_KeypadDecimal;
    case XKB_KEY_KP_Divide:
        return ImGuiKey_KeypadDivide;
    case XKB_KEY_KP_Multiply:
        return ImGuiKey_KeypadMultiply;
    case XKB_KEY_KP_Subtract:
        return ImGuiKey_KeypadSubtract;
    case XKB_KEY_KP_Add:
        return ImGuiKey_KeypadAdd;
    case XKB_KEY_KP_Enter:
        return ImGuiKey_KeypadEnter;
    case XKB_KEY_KP_Equal:
        return ImGuiKey_KeypadEqual;
    default:
        return ImGuiKey_None;
    }
}

Window::Window(Display& display)
    :_display(display)
{
    static constexpr wl_surface_listener wl_surface_listener {
        .enter = [](void *, wl_surface *, wl_output *) noexcept {

        },
        .leave = [](void *, wl_surface *, wl_output *) noexcept {
            
        },
        .preferred_buffer_scale = [](void *data, wl_surface *, int32_t factor){
            auto& self = *static_cast<Window *>(data);

            self._desired_integer_scale = factor;
        },
        .preferred_buffer_transform = [](void *, wl_surface *, uint32_t) noexcept {
            
        }
    };

    static constexpr xdg_surface_listener wm_surface_listener {
        .configure = [](void *data, xdg_surface *surface, uint32_t serial) noexcept {
            auto& self = *static_cast<Window *>(data);

            xdg_surface_ack_configure(surface, serial);

            if (self._desired_surface_size.first && self._desired_surface_size.second) {
                self._actual_surface_size = self._desired_surface_size;
            }
            self._desired_surface_size = { 0, 0 };

            if (self._desired_fractional_scale) {
                self._actual_fractional_scale = self._desired_fractional_scale;
            }
            self._desired_fractional_scale = 0;

            if (self._desired_integer_scale) {
                self._actual_integer_scale = self._desired_integer_scale;
            }
            self._desired_integer_scale = 0;

            if (self._actual_fractional_scale) {
                wp_viewport_set_destination(self._viewport.get(),
                    self._actual_surface_size.first,
                    self._actual_surface_size.second);
                wl_surface_set_buffer_scale(self._surface.get(), 1);
            } else if (self._actual_integer_scale) {
                wl_surface_set_buffer_scale(self._surface.get(), self._actual_integer_scale);
            }

            ImGui::GetIO().DisplaySize = { 
                static_cast<float>(self._actual_surface_size.first),
                static_cast<float>(self._actual_surface_size.second)
            };
        }
    };

    static constexpr xdg_toplevel_listener toplevel_listener {
        .configure = [](void *data, xdg_toplevel *, int32_t width, int32_t height, wl_array *) noexcept {
            auto& self = *static_cast<Window*>(data);

            self._desired_surface_size = { width, height };
        },
        .close = [](void *data, xdg_toplevel *) noexcept {
            auto& self = *static_cast<Window*>(data);

            self._closed = true;
        },
        .configure_bounds = [](void *, xdg_toplevel *, int32_t, int32_t) noexcept {
            
        },
        .wm_capabilities = [](void *, xdg_toplevel *, wl_array *) noexcept {
            
        }
    };

    static constexpr wp_fractional_scale_v1_listener fractional_scale_listener {
        .preferred_scale = [](void *data, wp_fractional_scale_v1 *, uint32_t scale) noexcept {
            auto& self = *static_cast<Window*>(data);

            self._desired_fractional_scale = scale;
        }
    };

    static constexpr zxdg_toplevel_decoration_v1_listener toplevel_decoration_listener {
        .configure = [](void *data, zxdg_toplevel_decoration_v1 *, uint32_t mode) noexcept {
            auto& self = *static_cast<Window*>(data);

            switch (mode) {
            case ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE:
                self._has_server_decorations = false;
                if (!self._fullscreen) {
                    self.toggle_fullscreen();
                }
                break;
            case ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE:
                self._has_server_decorations = true;
                break;
            default:
                break;
            }
        }
    };

    _surface.reset(wl_compositor_create_surface(_display._compositor.get()));
    wl_surface_add_listener(_surface.get(), &wl_surface_listener, this);

    _wm_surface.reset(xdg_wm_base_get_xdg_surface(_display._wm_base.get(), _surface.get()));
    xdg_surface_add_listener(_wm_surface.get(), &wm_surface_listener, this);

    _toplevel.reset(xdg_surface_get_toplevel(_wm_surface.get()));
    xdg_toplevel_add_listener(_toplevel.get(), &toplevel_listener, this);
    xdg_toplevel_set_min_size(_toplevel.get(), DEFAULT_WIDTH, DEFAULT_HEIGHT);
    xdg_toplevel_set_title(_toplevel.get(), WINDOW_TITLE);

    if (_display._content_type_manager) {
        _content_type.reset(wp_content_type_manager_v1_get_surface_content_type(display._content_type_manager.get(), _surface.get()));
        wp_content_type_v1_set_content_type(_content_type.get(), WP_CONTENT_TYPE_V1_TYPE_GAME);
    }

    if (_display._has_fractional_scale) {
        _fractional_scale.reset(wp_fractional_scale_manager_v1_get_fractional_scale(display._fractional_scale_manager.get(), _surface.get()));
        wp_fractional_scale_v1_add_listener(_fractional_scale.get(), &fractional_scale_listener, this);

        _viewport.reset(wp_viewporter_get_viewport(display._viewporter.get(), _surface.get()));
    }

    if (_display._decoration_manager) {
        _toplevel_decoration.reset(zxdg_decoration_manager_v1_get_toplevel_decoration(display._decoration_manager.get(), _toplevel.get()));
        zxdg_toplevel_decoration_v1_add_listener(_toplevel_decoration.get(), &toplevel_decoration_listener, this);
        zxdg_toplevel_decoration_v1_set_mode(_toplevel_decoration.get(), ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }

    _closed = false;
    _fullscreen = false;
    _has_server_decorations = !!_display._decoration_manager;

    _actual_integer_scale = 0;
    _desired_integer_scale = 0;

    if (display._has_fractional_scale) {
        _actual_fractional_scale = DEFAULT_SCALE_DPI;
    } else {
        _actual_fractional_scale = 0;
    }
    _desired_fractional_scale = 0;

    _actual_surface_size = {DEFAULT_WIDTH, DEFAULT_HEIGHT};
    _desired_surface_size = {0, 0};

    if (!_has_server_decorations) {
        toggle_fullscreen();
    }

    wl_surface_commit(_surface.get());
    wl_display_roundtrip(_display._display.get());
}

void Window::key_down(xkb_keysym_t keysym) noexcept {
    const auto key = xkb_to_imgui_key(keysym);
    if (key) {
        printf("0x%x DOWN\n", key);
        ImGui::GetIO().AddKeyEvent(key, true);
    }
}

void Window::key_up(xkb_keysym_t keysym) noexcept {
    const auto key = xkb_to_imgui_key(keysym);
    if (key) {
        printf("0x%x UP\n", key);
        ImGui::GetIO().AddKeyEvent(key, false);
    }
}

void Window::key_modifiers(bool shift, bool ctrl, bool alt) noexcept {
    if (shift) {
        ImGui::GetIO().AddKeyEvent(ImGuiMod_Shift, true);
    } else {
        ImGui::GetIO().AddKeyEvent(ImGuiMod_Shift, false);
    }
    if (ctrl) {
        ImGui::GetIO().AddKeyEvent(ImGuiMod_Ctrl, true);
    } else {
        ImGui::GetIO().AddKeyEvent(ImGuiMod_Ctrl, false);
    }
    if (alt) {
        ImGui::GetIO().AddKeyEvent(ImGuiMod_Alt, true);
    } else {
        ImGui::GetIO().AddKeyEvent(ImGuiMod_Alt, false);
    }
}

void Window::pointer_click(uint32_t button, wl_pointer_button_state state) noexcept {
    ImGuiButtonFlags imgui_button;
    switch (button) {
    case BTN_LEFT:
        imgui_button = ImGuiMouseButton_Left;
        break;
    case BTN_RIGHT:
        imgui_button = ImGuiMouseButton_Right;
        break;
    case BTN_MIDDLE:
        imgui_button = ImGuiMouseButton_Middle;
        break;
    default:
        return; // Unknown button
    }

    bool is_pressed;
    switch (state) {
    case WL_POINTER_BUTTON_STATE_PRESSED:
        is_pressed = true;
        break;
    case WL_POINTER_BUTTON_STATE_RELEASED:
        is_pressed = false;
        break;
    default:
        return; // Unknown state
    }

    printf("%d\n", imgui_button);
    ImGui::GetIO().AddMouseButtonEvent(imgui_button, is_pressed);
}

void Window::pointer_motion(float x, float y) noexcept {
    ImGui::GetIO().AddMousePosEvent(x, y);
}

void Window::text(const std::string& str) const noexcept {
    ImGui::GetIO().AddInputCharactersUTF8(str.c_str());
}

wl_display *Window::display() noexcept {
    return _display._display.get();
}

uint32_t Window::buffer_scale() const noexcept {
    if (_actual_fractional_scale) return _actual_fractional_scale;
    if (_actual_integer_scale) return static_cast<uint32_t>(_actual_integer_scale) * DEFAULT_SCALE_DPI;
    return DEFAULT_SCALE_DPI;
}

std::pair<uint32_t, uint32_t> Window::buffer_size() const noexcept {
    return {
        static_cast<uint32_t>(_actual_surface_size.first) * buffer_scale() / DEFAULT_SCALE_DPI,
        static_cast<uint32_t>(_actual_surface_size.second) * buffer_scale() / DEFAULT_SCALE_DPI
    };
}

bool Window::should_close() const noexcept {
    return _closed;
}

wl_surface *Window::surface() noexcept {
    return _surface.get();
}

void Window::update_ui() {
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();
    ImGui::Render();
}

void Window::toggle_fullscreen() noexcept {
    if (!_fullscreen) {
        xdg_toplevel_set_fullscreen(_toplevel.get(), nullptr);
        _fullscreen = true;
    } else if (_has_server_decorations) {
        xdg_toplevel_unset_fullscreen(_toplevel.get());
        _fullscreen = false;
    }
}
