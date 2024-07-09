#include "Pointer.hpp"

#include "Display.hpp"
#include "Seat.hpp"

static constexpr char DEFAULT_CURSOR_NAME[] = "default";

Pointer::Pointer(Seat& seat, wl_pointer* pointer)
    :_seat(seat), _pointer(pointer)
{
    static constexpr wl_pointer_listener pointer_listener {
        .enter = [](void *data, wl_pointer *, uint32_t serial, wl_surface *, wl_fixed_t, wl_fixed_t){
            auto& self = *static_cast<Pointer *>(data);

            wl_pointer_set_cursor(self._pointer.get(), serial, self._cursor_surface.get(), static_cast<int32_t>(self._cursor_image->hotspot_x), static_cast<int32_t>(self._cursor_image->hotspot_y));
        },
        .leave = [](void *, wl_pointer *, uint32_t, wl_surface *){},
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

    wl_pointer_add_listener(_pointer.get(), &pointer_listener, this);

    const auto& cursor = *wl_cursor_theme_get_cursor(_seat._display._cursor_theme.get(), DEFAULT_CURSOR_NAME);

    _cursor_image = cursor.images[0];
    _cursor_surface.reset(wl_compositor_create_surface(_seat._display._compositor.get()));
    wl_surface_attach(_cursor_surface.get(), wl_cursor_image_get_buffer(_cursor_image), 0, 0);
    wl_surface_commit(_cursor_surface.get());
}
