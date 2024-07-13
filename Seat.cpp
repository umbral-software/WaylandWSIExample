#include "Seat.hpp"

Seat::Seat(Display& display, wl_seat *seat)
    :_display(display), _seat(seat)
{
    static constexpr wl_seat_listener seat_listener {
        .capabilities = [](void *data, wl_seat *, uint32_t capabilities){
            auto& self = *static_cast<Seat *>(data);

            if (WL_SEAT_CAPABILITY_KEYBOARD & capabilities) {
                if (!self._keyboard) {
                    self._keyboard.emplace(self);
                }
            } else {
                self._keyboard.reset();
            }

            if (WL_SEAT_CAPABILITY_POINTER & capabilities) {
                if (!self._pointer) {
                    self._pointer.emplace(self);
                }
            } else {
                self._pointer.reset();
            }
        },
        .name = [](void *, wl_seat *, const char *){

        }
    };

    wl_seat_add_listener(_seat.get(), &seat_listener, this);
}
