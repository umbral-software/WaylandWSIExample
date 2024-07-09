#include "Keyboard.hpp"

#include "MappableFd.hpp"
#include "Seat.hpp"

Keyboard::Keyboard(Seat& seat) {
    static constexpr wl_keyboard_listener keyboard_listener {
        .keymap = [](void *, wl_keyboard *, uint32_t, int fd, uint32_t len){
            MappableFd file(fd, len);
            
        },
        .enter = [](void *, wl_keyboard *, uint32_t, wl_surface *, wl_array *){},
        .leave = [](void *, wl_keyboard *, uint32_t, wl_surface *){},
        .key = [](void *, wl_keyboard *, uint32_t, uint32_t, uint32_t, uint32_t){},
        .modifiers = [](void *, wl_keyboard *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t){},
        .repeat_info = [](void *, wl_keyboard *, int32_t, int32_t){}
    };

    _keyboard.reset(wl_seat_get_keyboard(seat._seat.get()));
    wl_keyboard_add_listener(_keyboard.get(), &keyboard_listener, this);
}
