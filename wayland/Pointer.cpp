#include "Pointer.hpp"

#include "Display.hpp"
#include "Seat.hpp"

Pointer::Pointer(Seat& seat)
    :_display(seat._display)
    ,_focus(nullptr)
{
    static constexpr wl_pointer_listener pointer_listener {
        .enter = [](void *data, wl_pointer *, uint32_t serial, wl_surface *surface, wl_fixed_t, wl_fixed_t) noexcept {
            auto& self = *static_cast<Pointer *>(data);

            if (surface) {
                self._focus = static_cast<Window *>(wl_surface_get_user_data(surface));
                self._cursor->set_pointer(serial);
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

    _cursor = _display._cursor_manager->get_cursor(_pointer.get());
}
