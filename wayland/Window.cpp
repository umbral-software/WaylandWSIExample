#include "Window.hpp"

#include "Display.hpp"

#include <cstring>

static constexpr uint32_t DEFAULT_HEIGHT = 600;
static constexpr uint32_t DEFAULT_WIDTH = 800;
static constexpr char WINDOW_TITLE[] = "vfighter";

Window::Window(Display& display)
    :_display(display)
{
    static constexpr xdg_surface_listener wm_surface_listener {
        .configure = [](void *data, xdg_surface *surface, uint32_t serial) noexcept {
            auto& self = *static_cast<Window *>(data);
            xdg_surface_ack_configure(surface, serial);
            self._actual_size = {
                std::max(DEFAULT_WIDTH, self._desired_size.first),
                std::max(DEFAULT_HEIGHT, self._desired_size.second)
            };
        }
    };

    static constexpr xdg_toplevel_listener toplevel_listener {
        .configure = [](void *data, xdg_toplevel *, int32_t width, int32_t height, wl_array *) noexcept {
            auto& self = *static_cast<Window*>(data);

            self._desired_size = { width, height };
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

    static constexpr zxdg_toplevel_decoration_v1_listener toplevel_decoration_listener {
        .configure = [](void *data, zxdg_toplevel_decoration_v1 *, uint32_t mode) {
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
    wl_surface_set_user_data(_surface.get(), this);

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

    if (_display._decoration_manager) {
        _toplevel_decoration.reset(zxdg_decoration_manager_v1_get_toplevel_decoration(display._decoration_manager.get(), _toplevel.get()));
        zxdg_toplevel_decoration_v1_add_listener(_toplevel_decoration.get(), &toplevel_decoration_listener, this);
        zxdg_toplevel_decoration_v1_set_mode(_toplevel_decoration.get(), ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }

    _closed = false;
    _fullscreen = false;
    _has_server_decorations = !!_display._decoration_manager;

    _actual_size = {DEFAULT_WIDTH, DEFAULT_HEIGHT};
    _desired_size = {0, 0};

    if (!_has_server_decorations) {
        toggle_fullscreen();
    }

    wl_surface_commit(_surface.get());
}

void Window::keysym(uint32_t keysym, bool, bool, bool alt) noexcept {
    switch (keysym) {
    case XKB_KEY_Return:
        if (alt) {
            toggle_fullscreen();
        } else {
            std::putchar('\n');
        }
        break;
    case XKB_KEY_Escape:
        _closed = true;
        break;
    default:
        break;
    }
}

void Window::text(std::string_view str) const noexcept {
    fwrite(str.data(), 1, str.size(), stdout);
}

wl_display *Window::display() noexcept {
    return _display._display.get();
}

bool Window::should_close() const noexcept {
    return _closed;
}

std::pair<uint32_t, uint32_t> Window::size() const noexcept {
    return _actual_size;
}

wl_surface *Window::surface() noexcept {
    return _surface.get();
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
