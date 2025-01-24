#include "Window.hpp"

#include "Display.hpp"

#include <cstring>
#include <utility>
#include <wayland-client-protocol.h>

static constexpr int32_t DEFAULT_HEIGHT = 600;
static constexpr int32_t DEFAULT_WIDTH = 800;
static constexpr char WINDOW_TITLE[] = "Wayland Example";

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

            if (self._desired_surface_bounds.has_value()) {
                self._actual_surface_bounds = self._desired_surface_bounds.value();
            }
            self._desired_surface_bounds = std::nullopt;

            if (self._desired_surface_size.has_value()) {
                self._actual_surface_size = self._desired_surface_size.value();
            }
            self._desired_surface_size = std::nullopt;

            if (self._desired_fractional_scale.has_value()) {
                self._actual_fractional_scale = self._desired_fractional_scale.value();
            }
            self._desired_fractional_scale = std::nullopt;

            if (self._desired_integer_scale.has_value()) {
                self._actual_integer_scale = self._desired_integer_scale.value();
            }
            self._desired_integer_scale = std::nullopt;

            if (self._actual_fractional_scale) {
                const auto size = self.surface_size();
                wp_viewport_set_destination(self._viewport.get(), size.first, size.second);
                wl_surface_set_buffer_scale(self._surface.get(), 1);
            } else if (self._actual_integer_scale) {
                wl_surface_set_buffer_scale(self._surface.get(), self._actual_integer_scale);
            } else {
                wl_surface_set_buffer_scale(self._surface.get(), 1);
            }
        }
    };

    static constexpr xdg_toplevel_listener toplevel_listener {
        .configure = [](void *data, xdg_toplevel *, int32_t width, int32_t height, wl_array *states) noexcept {
            auto& self = *static_cast<Window*>(data);

            self._desired_surface_size = { width, height };
            self._fullscreen = false;
            self._maximized = false;

            for (const auto *pstate = (int32_t *)states->data; states->size != 0 && (const char *)pstate < ((const char *) states->data + states->size); pstate++) {
                switch (*pstate) {
                case XDG_TOPLEVEL_STATE_MAXIMIZED:
                    self._maximized = true;
                    break;
                case XDG_TOPLEVEL_STATE_FULLSCREEN:
                    self._fullscreen = true;
                    break;
                default:
                    break;
                }
            }
        },
        .close = [](void *data, xdg_toplevel *) noexcept {
            auto& self = *static_cast<Window*>(data);

            self._closed = true;
        },
        .configure_bounds = [](void *data, xdg_toplevel *, int32_t width, int32_t height) noexcept {
            auto& self = *static_cast<Window*>(data);
                        
            self._desired_surface_bounds = { width, height };
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
    _maximized = false;
    _has_server_decorations = !!_display._decoration_manager;

    _actual_integer_scale = 0;

    if (display._has_fractional_scale) {
        _actual_fractional_scale = DEFAULT_SCALE_DPI;
    } else {
        _actual_fractional_scale = 0;
    }

    _actual_surface_bounds = { INT32_MAX, INT32_MAX };
    _actual_surface_size = {DEFAULT_WIDTH, DEFAULT_HEIGHT};

    if (!_has_server_decorations) {
        toggle_fullscreen();
    }

    wl_surface_commit(_surface.get());
    wl_display_roundtrip(_display._display.get());
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
    const auto scale = static_cast<double>(buffer_scale()) / static_cast<double>(DEFAULT_SCALE_DPI);
    const auto size = surface_size();

    return {
        size.first * scale,
        size.second * scale
    };
}

std::pair<uint32_t, uint32_t> Window::surface_size() const noexcept {
    auto size = _actual_surface_size;
    if (size == std::make_pair(0,0)) {
        size = std::make_pair(
            std::min(DEFAULT_WIDTH, _actual_surface_bounds.first),
            std::min(DEFAULT_HEIGHT, _actual_surface_bounds.second)
        );
    }
    return size;
}

bool Window::should_close() const noexcept {
    return _closed;
}

wl_surface *Window::surface() noexcept {
    return _surface.get();
}

void Window::toggle_fullscreen() noexcept {
    if (!_fullscreen) {
        xdg_toplevel_set_fullscreen(_toplevel.get(), nullptr);
    } else if (_has_server_decorations) {
        xdg_toplevel_unset_fullscreen(_toplevel.get());
    }
}
