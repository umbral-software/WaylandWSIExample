#include "Seat.hpp"

#include <sys/mman.h>

Seat::Seat(Display& display, wl_seat *seat)
    :_display(display), _seat(seat)
{
    static constexpr wl_keyboard_listener keyboard_listener {
        .keymap = [](void *, wl_keyboard *, uint32_t, int fd, uint32_t len){
            const auto pmap = mmap(nullptr, len, PROT_READ, MAP_PRIVATE, fd, 0);
            puts(static_cast<const char *>(pmap));
            munmap(pmap, len);
            close(fd);
        },
        .enter = [](void *, wl_keyboard *, uint32_t, wl_surface *, wl_array *){},
        .leave = [](void *, wl_keyboard *, uint32_t, wl_surface *){},
        .key = [](void *, wl_keyboard *, uint32_t, uint32_t, uint32_t, uint32_t){},
        .modifiers = [](void *, wl_keyboard *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t){},
        .repeat_info = [](void *, wl_keyboard *, int32_t, int32_t){}
    };

    static constexpr wl_seat_listener seat_listener {
        .capabilities = [](void *data, wl_seat *, uint32_t capabilities){
            auto& self = *static_cast<Seat *>(data);

            if (WL_SEAT_CAPABILITY_KEYBOARD & capabilities && !self._keyboard) {
                self._keyboard.reset(wl_seat_get_keyboard(self._seat.get()));
                wl_keyboard_add_listener(self._keyboard.get(), &keyboard_listener, data);
            }
            if (WL_SEAT_CAPABILITY_POINTER & capabilities && !self._pointer) {
                self._pointer.emplace(self, wl_seat_get_pointer(self._seat.get()));
            }
        },
        .name = [](void *, wl_seat *, const char *){

        }
    };

    wl_seat_add_listener(_seat.get(), &seat_listener, this);
    
}
