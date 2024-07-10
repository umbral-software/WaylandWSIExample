#include "Window.hpp"

#include "Display.hpp"

#include <cstring>

static constexpr int32_t DEFAULT_HEIGHT = 600;
static constexpr int32_t DEFAULT_WIDTH = 800;
static constexpr char WINDOW_TITLE[] = "vfighter";

Window::Window(Display& display)
    :_display(display)
{
    static constexpr wl_surface_listener surface_listener {
        .enter = [](void *, wl_surface *, wl_output *) noexcept {
            
        },
        .leave = [](void *, wl_surface *, wl_output *) noexcept {
            
        },
        .preferred_buffer_scale = [](void *, wl_surface *, int32_t) noexcept {
            
        },
        .preferred_buffer_transform = [](void *, wl_surface *, uint32_t) noexcept {
            
        },
    };

    static constexpr xdg_surface_listener wm_surface_listener {
        .configure = [](void *data, xdg_surface *surface, uint32_t serial) noexcept {
            auto& self = *static_cast<Window *>(data);
            xdg_surface_ack_configure(surface, serial);
            wl_surface_commit(self._surface.get());
        }
    };

    static constexpr xdg_toplevel_listener toplevel_listener {
        .configure = [](void *, xdg_toplevel *, int32_t, int32_t, wl_array *) noexcept {

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
                xdg_toplevel_set_fullscreen(self._toplevel.get(), nullptr);
                break;
            case ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE:
                break;
            default:
                break;
            }
        }
    };

    _surface.reset(wl_compositor_create_surface(_display._compositor.get()));
    wl_surface_add_listener(_surface.get(), &surface_listener, this);

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
    } else {
        xdg_toplevel_set_fullscreen(_toplevel.get(), nullptr);
    }

    wl_surface_commit(_surface.get());
}

wl_display *Window::display() noexcept {
    return _display._display.get();
}

wl_surface *Window::surface() noexcept {
    return _surface.get();
}


bool Window::should_close() const noexcept {
    return _closed;
}
