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

                wl_pointer_set_cursor(self._pointer.get(), serial, self._cursor_surface.get(), static_cast<int32_t>(self._cursor_image->hotspot_x), static_cast<int32_t>(self._cursor_image->hotspot_y));
            }
        },
        .leave = [](void *data, wl_pointer *, uint32_t, wl_surface *) noexcept {
            auto& self = *static_cast<Pointer *>(data);

            self._focus = nullptr;
        },
        .motion  = [](void *, wl_pointer *, uint32_t, wl_fixed_t, wl_fixed_t){},
        .button = [](void *, wl_pointer *, uint32_t, uint32_t, uint32_t, uint32_t){},
        .axis = [](void *, wl_pointer *, uint32_t, uint32_t, wl_fixed_t){},
        .frame = [](void *, wl_pointer *){},
        .axis_source = [](void *, wl_pointer *, uint32_t){},
        .axis_stop = [](void *, wl_pointer *, uint32_t, uint32_t){},
        .axis_discrete = [](void *, wl_pointer *, uint32_t, int32_t){},
        .axis_value120 = [](void *, wl_pointer *, uint32_t, int32_t){},
        .axis_relative_direction = [](void *, wl_pointer *, uint32_t, uint32_t){},
    };

    _pointer.reset(wl_seat_get_pointer(seat._seat.get()));
    wl_pointer_add_listener(_pointer.get(), &pointer_listener, this);

    const auto& cursor = *wl_cursor_theme_get_cursor(_display._cursor_theme.get(), DEFAULT_CURSOR_NAME);

    _cursor_image = cursor.images[0];
    _cursor_surface.reset(wl_compositor_create_surface(_display._compositor.get()));
    wl_surface_attach(_cursor_surface.get(), wl_cursor_image_get_buffer(_cursor_image), 0, 0);
    wl_surface_commit(_cursor_surface.get());
}
