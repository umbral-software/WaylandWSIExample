#include "Pointer.hpp"

#include "Display.hpp"
#include "Seat.hpp"

static constexpr char DEFAULT_CURSOR_NAME[] = "default";

Pointer::Pointer(Seat& seat)
    :_display(seat._display)
    ,_focus(nullptr)
{
    static constexpr wl_pointer_listener pointer_listener {
        .enter = [](void *data, wl_pointer *, uint32_t serial, wl_surface *surface, wl_fixed_t, wl_fixed_t) noexcept {
            auto& self = *static_cast<Pointer *>(data);

            if (surface) {
                self._focus = static_cast<Window *>(wl_surface_get_user_data(surface));

                if (self._cursor_shape_device) {
                    wp_cursor_shape_device_v1_set_shape(self._cursor_shape_device.get(), serial, WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_WAIT);
                } else {
                    wl_pointer_set_cursor(self._pointer.get(), serial, self._cursor_surface.get(), static_cast<int32_t>(self._cursor_image->hotspot_x), static_cast<int32_t>(self._cursor_image->hotspot_y));
                }
            }
        },
        .leave = [](void *data, wl_pointer *, uint32_t, wl_surface *) noexcept {
            auto& self = *static_cast<Pointer *>(data);

            self._focus = nullptr;
        },
        .motion  = [](void *, wl_pointer *, uint32_t, wl_fixed_t, wl_fixed_t) noexcept {},
        .button = [](void *, wl_pointer *, uint32_t, uint32_t, uint32_t, uint32_t) noexcept {},
        .axis = [](void *, wl_pointer *, uint32_t, uint32_t, wl_fixed_t) noexcept {},
        .frame = [](void *, wl_pointer *) noexcept {},
        .axis_source = [](void *, wl_pointer *, uint32_t) noexcept {},
        .axis_stop = [](void *, wl_pointer *, uint32_t, uint32_t) noexcept {},
        .axis_discrete = [](void *, wl_pointer *, uint32_t, int32_t) noexcept {},
        .axis_value120 = [](void *, wl_pointer *, uint32_t, int32_t) noexcept {},
        .axis_relative_direction = [](void *, wl_pointer *, uint32_t, uint32_t) noexcept {},
    };

    _pointer.reset(wl_seat_get_pointer(seat._seat.get()));
    wl_pointer_add_listener(_pointer.get(), &pointer_listener, this);

    const auto& cursor = *wl_cursor_theme_get_cursor(_display._cursor_theme.get(), DEFAULT_CURSOR_NAME);

    _cursor_image = cursor.images[0];
    _cursor_surface.reset(wl_compositor_create_surface(_display._compositor.get()));
    wl_surface_attach(_cursor_surface.get(), wl_cursor_image_get_buffer(_cursor_image), 0, 0);
    wl_surface_commit(_cursor_surface.get());

    if (_display._cursor_shape_manager) {
        _cursor_shape_device.reset(wp_cursor_shape_manager_v1_get_pointer(_display._cursor_shape_manager.get(), _pointer.get()));
    }
}
