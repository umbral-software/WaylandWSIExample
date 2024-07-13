#include "Display.hpp"

#include <poll.h>

#include <cstring>

static constexpr int DEFAULT_CURSOR_SIZE = 16;

static constexpr uint32_t DESIRED_WL_COMPOSITOR_VERSION = 4;
static constexpr uint32_t DESIRED_WL_SEAT_VERSION = 5;
static constexpr uint32_t DESIRED_WL_SHM_VERSION = 1;
static constexpr uint32_t DESIRED_WP_CONTENT_TYPE_V1_VERSION = 1;
static constexpr uint32_t DESIRED_XDG_DECORATION_V1_VERSION = 1;
static constexpr uint32_t DESIRED_XDG_SHELL_VERSION = 2;

static short poll_single(int fd, short events, int timeout) {
    pollfd pfd { .fd = fd, .events = events, .revents = 0 };
    if (0 > poll(&pfd, 1, timeout)) {
        throw std::runtime_error("poll() failed");
    }

    return pfd.revents;
}

Display::Display() {
    static constexpr wl_registry_listener registry_listener {
        .global = [](void *data, wl_registry *registry, uint32_t name, const char *interface, uint32_t version) noexcept {
            auto& self = *static_cast<Display*>(data);

            if (!strcmp(wl_compositor_interface.name, interface)
                && version >= DESIRED_WL_COMPOSITOR_VERSION)
            {
                self._compositor.reset(static_cast<wl_compositor *>(wl_registry_bind(registry, name, &wl_compositor_interface, DESIRED_WL_COMPOSITOR_VERSION)));
            }
            else if (!strcmp(wl_seat_interface.name, interface)
                && version >= DESIRED_WL_SEAT_VERSION)
            {
                self._seats.emplace_front(self, static_cast<wl_seat *>(wl_registry_bind(registry, name, &wl_seat_interface, DESIRED_WL_SEAT_VERSION)));
            }
            else if (!strcmp(wl_shm_interface.name, interface)
                && version >= DESIRED_WL_SHM_VERSION)
            {
                self._shm.reset(static_cast<wl_shm *>(wl_registry_bind(registry, name, &wl_shm_interface, DESIRED_WL_SHM_VERSION)));
            }
            else if (!strcmp(xdg_wm_base_interface.name, interface)
                && version >= DESIRED_XDG_SHELL_VERSION)
            {
                self._wm_base.reset(static_cast<xdg_wm_base *>(wl_registry_bind(registry, name, &xdg_wm_base_interface, DESIRED_XDG_SHELL_VERSION)));
            }
            else if (!strcmp(wp_content_type_manager_v1_interface.name, interface)
                && version >= DESIRED_WP_CONTENT_TYPE_V1_VERSION)
            {
                self._content_type_manager.reset(static_cast<wp_content_type_manager_v1 *>(wl_registry_bind(registry, name, &wp_content_type_manager_v1_interface, DESIRED_WP_CONTENT_TYPE_V1_VERSION)));
            }
            else if (!strcmp(zxdg_decoration_manager_v1_interface.name, interface)
                && version >= DESIRED_XDG_DECORATION_V1_VERSION)
            {
                self._decoration_manager.reset(static_cast<zxdg_decoration_manager_v1 *>(wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, DESIRED_XDG_DECORATION_V1_VERSION)));
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

    _registry.reset(wl_display_get_registry(_display.get()));
    wl_registry_add_listener(_registry.get(), &registry_listener, this);
    wl_display_roundtrip(_display.get());

    if (!_compositor || !_shm || !_wm_base) {
        throw std::runtime_error("Missing required wayland globals");
    }

    xdg_wm_base_add_listener(_wm_base.get(), &wm_base_listener, this);

    const auto xcursor_size = getenv("XCURSOR_SIZE");
    const auto cursor_size = xcursor_size ? atoi(xcursor_size) : DEFAULT_CURSOR_SIZE;
    _cursor_theme.reset(wl_cursor_theme_load(nullptr, cursor_size, _shm.get()));

    _xkb_context.reset(xkb_context_new(XKB_CONTEXT_NO_FLAGS));
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
