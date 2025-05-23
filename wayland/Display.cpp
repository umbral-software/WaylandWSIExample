#include "Display.hpp"

#include "cursor/shape/ShapeCursorManager.hpp"
#include "cursor/theme/ThemeCursorManager.hpp"

#include <poll.h>

#include <cstring>

static constexpr uint32_t MINIMUM_WL_COMPOSITOR_VERSION = 4;
static constexpr uint32_t DESIRED_WL_COMPOSITOR_VERSION = 6;

static constexpr uint32_t MINIMUM_WL_SEAT_VERSION = 7;
static constexpr uint32_t DESIRED_WL_SEAT_VERSION = 7;

static constexpr uint32_t MINIMUM_WL_SHM_VERSION = 1;
static constexpr uint32_t DESIRED_WL_SHM_VERSION = 1;

static constexpr uint32_t MINIMUM_WP_CONTENT_TYPE_V1_VERSION = 1;
static constexpr uint32_t DESIRED_WP_CONTENT_TYPE_V1_VERSION = 1;

static constexpr uint32_t MINIMUM_WP_CURSOR_SHAPE_V1_VERSION = 1;
static constexpr uint32_t DESIRED_WP_CURSOR_SHAPE_V1_VERSION = 1;

static constexpr uint32_t MINIMUM_WP_FRACTIONAL_SCALE_V1_VERSION = 1;
static constexpr uint32_t DESIRED_WP_FRACTIONAL_SCALE_V1_VERSION = 1;

static constexpr uint32_t MINIMUM_WP_VIEWPORTER_VERSION = 1;
static constexpr uint32_t DESIRED_WP_VIEWPORTER_VERSION = 1;

static constexpr uint32_t MINIMUM_XDG_DECORATION_V1_VERSION = 1;
static constexpr uint32_t DESIRED_XDG_DECORATION_V1_VERSION = 1;

static constexpr uint32_t MINIMUM_XDG_SHELL_VERSION = 2;
static constexpr uint32_t DESIRED_XDG_SHELL_VERSION = 4;

static short poll_single(int fd, short events, int timeout) {
    pollfd pfd { .fd = fd, .events = events, .revents = 0 };
    if (0 > poll(&pfd, 1, timeout)) {
        throw std::runtime_error("poll() failed");
    }

    return pfd.revents;
}

template<typename T>
static T *do_bind(wl_registry *wl_registry, uint32_t name, uint32_t version, const wl_interface *interface, uint32_t desired_version) noexcept {
    return static_cast<T *>(wl_registry_bind(wl_registry, name, interface, std::min(version, desired_version)));
}

Display::Display() {
    static constexpr wl_registry_listener registry_listener {
        .global = [](void *data, wl_registry *wl_registry, uint32_t name, const char *interface, uint32_t version) noexcept {
            auto& self = *static_cast<Display*>(data);

            if (!strcmp(wl_compositor_interface.name, interface)
                && version >= MINIMUM_WL_COMPOSITOR_VERSION)
            {
                self._compositor.reset(do_bind<wl_compositor>(
                    wl_registry, name, version,
                    &wl_compositor_interface,
                    DESIRED_WL_COMPOSITOR_VERSION
                ));
            }
            else if (!strcmp(wl_seat_interface.name, interface)
                && version >= MINIMUM_WL_SEAT_VERSION)
            {
                self._seats.emplace_front(self, do_bind<wl_seat>(
                    wl_registry, name, version,
                    &wl_seat_interface,
                    DESIRED_WL_SEAT_VERSION
                ));
            }
            else if (!strcmp(wl_shm_interface.name, interface)
                && version >= MINIMUM_WL_SHM_VERSION)
            {
                self._shm.reset(do_bind<wl_shm>(
                    wl_registry, name, version,
                    &wl_shm_interface,
                    DESIRED_WL_SHM_VERSION
                ));
            }
            else if (!strcmp(wp_fractional_scale_manager_v1_interface.name, interface)
                && version >= MINIMUM_WP_FRACTIONAL_SCALE_V1_VERSION)
            {
                self._fractional_scale_manager.reset(do_bind<wp_fractional_scale_manager_v1>(
                    wl_registry, name, version,
                    &wp_fractional_scale_manager_v1_interface,
                    DESIRED_WP_FRACTIONAL_SCALE_V1_VERSION
                ));
            }
            else if (!strcmp(wp_viewporter_interface.name, interface)
                && version >= MINIMUM_WP_VIEWPORTER_VERSION)
            {
                self._viewporter.reset(do_bind<wp_viewporter>(
                    wl_registry, name, version,
                    &wp_viewporter_interface,
                    DESIRED_WP_VIEWPORTER_VERSION
                ));
            }
            else if (!strcmp(xdg_wm_base_interface.name, interface)
                && version >= MINIMUM_XDG_SHELL_VERSION)
            {
                self._wm_base.reset(do_bind<xdg_wm_base>(
                    wl_registry, name, version,
                    &xdg_wm_base_interface,
                    DESIRED_XDG_SHELL_VERSION
                ));
            }
            else if (!strcmp(wp_content_type_manager_v1_interface.name, interface)
                && version >= MINIMUM_WP_CONTENT_TYPE_V1_VERSION)
            {
                self._content_type_manager.reset(do_bind<wp_content_type_manager_v1>(
                    wl_registry, name, version,
                    &wp_content_type_manager_v1_interface,
                    DESIRED_WP_CONTENT_TYPE_V1_VERSION
                ));
            }
            else if (!strcmp(wp_cursor_shape_manager_v1_interface.name, interface)
                && version >= MINIMUM_WP_CURSOR_SHAPE_V1_VERSION)
            {
                self._cursor_manager = std::make_unique<ShapeCursorManager>(do_bind<wp_cursor_shape_manager_v1>(
                    wl_registry, name, version,
                    &wp_cursor_shape_manager_v1_interface,
                    DESIRED_WP_CURSOR_SHAPE_V1_VERSION
                ));
            }
            else if (!strcmp(xdg_wm_base_interface.name, interface)
                && version >= MINIMUM_XDG_SHELL_VERSION)
            {
                self._wm_base.reset(do_bind<xdg_wm_base>(
                    wl_registry, name, version,
                    &xdg_wm_base_interface,
                    DESIRED_XDG_SHELL_VERSION
                ));
            }
            else if (!strcmp(zxdg_decoration_manager_v1_interface.name, interface)
                && version >= MINIMUM_XDG_DECORATION_V1_VERSION)
            {
                self._decoration_manager.reset(do_bind<zxdg_decoration_manager_v1>(
                    wl_registry, name, version,
                    &zxdg_decoration_manager_v1_interface, DESIRED_XDG_DECORATION_V1_VERSION
                ));
            }
        },
        .global_remove = [](void *, wl_registry *, uint32_t) noexcept {
            
        }
    };

    static constexpr xdg_wm_base_listener wm_base_listener {
        .ping = [](void *, xdg_wm_base *wm_base, uint32_t serial) noexcept {
            xdg_wm_base_pong(wm_base, serial);
        }
    };

    _display.reset(wl_display_connect(nullptr));
    if (!_display) {
        throw std::runtime_error("No wayland compositor detected");
    }

    _registry.reset(wl_display_get_registry(_display.get()));
    wl_registry_add_listener(_registry.get(), &registry_listener, this);
    wl_display_roundtrip(_display.get());

    if (!_compositor || !_wm_base || (!_cursor_manager && !_shm)) {
        throw std::runtime_error("Missing required wayland globals");
    }

    xdg_wm_base_add_listener(_wm_base.get(), &wm_base_listener, this);

    if (!_cursor_manager) {
        _cursor_manager = std::make_unique<ThemeCursorManager>(_compositor.get(), _shm.get());
    }
    
    _xkb_context.reset(xkb_context_new(XKB_CONTEXT_NO_FLAGS));

    _has_fractional_scale = _fractional_scale_manager && _viewporter;
}

void Display::poll_events() {
    while (wl_display_prepare_read(_display.get())) {
        wl_display_dispatch_pending(_display.get());
    }

    while (wl_display_flush(_display.get()) < 0 && EAGAIN == errno) {
        poll_single(wl_display_get_fd(_display.get()), POLLOUT, -1);
    }

    if (POLLIN & poll_single(wl_display_get_fd(_display.get()), POLLIN, 0)) {
        wl_display_read_events(_display.get());
        wl_display_dispatch_pending(_display.get());
    } else {
        wl_display_cancel_read(_display.get());
    }

    if (wl_display_get_error(_display.get())) {
        throw std::runtime_error("Wayland protocol error");
    }    
}
