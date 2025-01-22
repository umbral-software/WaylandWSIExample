#pragma once

#include "wayland-content-type-client-protocol.h"
#include "wayland-cursor-shape-client-protocol.h"
#include "wayland-fractional-scale-client-protocol.h"
#include "wayland-viewporter-client-protocol.h"
#include "wayland-xdg-decoration-client-protocol.h"
#include "wayland-xdg-shell-client-protocol.h"

#include <wayland-cursor.h>

#include <memory>

struct WaylandDeleter {
    void operator()(wl_buffer *wl_buffer) const noexcept {
        wl_buffer_destroy(wl_buffer);
    }

    void operator()(wl_compositor *wl_compositor) const noexcept {
        wl_compositor_destroy(wl_compositor);
    }

    void operator()(wl_cursor_theme *wl_cursor_theme) const noexcept {
        wl_cursor_theme_destroy(wl_cursor_theme);
    }

    void operator()(wl_display *display) const noexcept {
        wl_display_disconnect(display);
    }

    void operator()(wl_keyboard *wl_keyboard) const noexcept {
        wl_keyboard_release(wl_keyboard);
    }

    void operator()(wl_pointer *wl_pointer) const noexcept {
        wl_pointer_release(wl_pointer);
    }

    void operator()(wl_registry *wl_registry) const noexcept {
        wl_registry_destroy(wl_registry);
    }

    void operator()(wl_seat *wl_seat) const noexcept {
        wl_seat_release(wl_seat);
    }

    void operator()(wl_shm *wl_shm) const noexcept {
        wl_shm_destroy(wl_shm);
    }

    void operator()(wl_shm_pool *wl_shm_pool) const noexcept {
        wl_shm_pool_destroy(wl_shm_pool);
    }

    void operator()(wl_surface *wl_surface) const noexcept {
        wl_surface_destroy(wl_surface);
    }

    void operator()(wl_touch *wl_touch) const noexcept {
        wl_touch_destroy(wl_touch);
    }

    void operator()(wp_content_type_manager_v1 *wp_content_type_manager_v1) const noexcept {
        wp_content_type_manager_v1_destroy(wp_content_type_manager_v1);
    }

    void operator()(wp_content_type_v1 *wp_content_type_v1) const noexcept {
        wp_content_type_v1_destroy(wp_content_type_v1);
    }

    void operator()(wp_cursor_shape_device_v1 *wp_cursor_shape_device_v1) const noexcept {
        wp_cursor_shape_device_v1_destroy(wp_cursor_shape_device_v1);
    }


    void operator()(wp_fractional_scale_v1 *wp_fractional_scale_v1) const noexcept {
        wp_fractional_scale_v1_destroy(wp_fractional_scale_v1);
    }

    void operator()(wp_fractional_scale_manager_v1 *wp_fractional_scale_manager_v1) const noexcept {
        wp_fractional_scale_manager_v1_destroy(wp_fractional_scale_manager_v1);
    }


    void operator()(wp_viewport *wp_viewport) const noexcept {
        wp_viewport_destroy(wp_viewport);
    }

    void operator()(wp_viewporter *wp_viewporter) const noexcept {
        wp_viewporter_destroy(wp_viewporter);
    }

    void operator()(wp_cursor_shape_manager_v1 *wp_cursor_shape_manager_v1) const noexcept {
        wp_cursor_shape_manager_v1_destroy(wp_cursor_shape_manager_v1);
    }

    void operator()(xdg_surface *xdg_surface) const noexcept {
        xdg_surface_destroy(xdg_surface);
    }

    void operator()(xdg_toplevel *xdg_toplevel) const noexcept {
        xdg_toplevel_destroy(xdg_toplevel);
    }

    void operator()(xdg_wm_base *xdg_wm_base) const noexcept {
        xdg_wm_base_destroy(xdg_wm_base);
    }

    void operator()(zxdg_decoration_manager_v1 *zxdg_decoration_manager_v1) const noexcept {
        zxdg_decoration_manager_v1_destroy(zxdg_decoration_manager_v1);
    }

    void operator()(zxdg_toplevel_decoration_v1 *zxdg_toplevel_decoration_v1) const noexcept {
        zxdg_toplevel_decoration_v1_destroy(zxdg_toplevel_decoration_v1);
    }
};

template<typename T>
concept WaylandDeletable = requires(T *t, const WaylandDeleter& d) {
    requires noexcept(d(t));
};

template<WaylandDeletable T>
using WaylandPointer = std::unique_ptr<T, WaylandDeleter>;
