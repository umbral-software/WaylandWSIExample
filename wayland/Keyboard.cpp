#include "Keyboard.hpp"

#include "Display.hpp"
#include "MappedFd.hpp"
#include "Seat.hpp"
#include "Window.hpp"

#include <xkbcommon/xkbcommon-keysyms.h>

static constexpr uint32_t XKB_EVDEV_OFFSET = 8;

Keyboard::Keyboard(Seat& seat)
    :_display(seat._display)
    ,_focus(nullptr)
{
    static constexpr wl_keyboard_listener keyboard_listener {
        .keymap = [](void *data, wl_keyboard *, uint32_t format, int fd, uint32_t size) noexcept {
            auto& self = *reinterpret_cast<Keyboard *>(data);
            // Note: fd is always valid, but may be of size 0
            MappedFd file(fd, size);

            switch (format) {
            case WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1: {
                self._keymap.reset(xkb_keymap_new_from_buffer(self._display._xkb_context.get(), static_cast<const char *>(file.map()), size, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS));
                break;
            }
            default:
                std::fprintf(stderr, "Unknown keymap type %d\n", format);
                break;
            }
        },
        .enter = [](void *data, wl_keyboard *, uint32_t, wl_surface *surface, wl_array *) noexcept {
            auto& self = *reinterpret_cast<Keyboard *>(data);
            if (surface) {
                self._focus = static_cast<Window *>(wl_surface_get_user_data(surface));

                if (self._keymap) {
                    self._state.reset(xkb_state_new(self._keymap.get()));
                }
            }
        },
        .leave = [](void *data, wl_keyboard *, uint32_t, wl_surface *) noexcept {
            auto& self = *reinterpret_cast<Keyboard *>(data);

            self._state.reset();
            self._focus = nullptr;           
        },
        .key = [](void *data, wl_keyboard *, uint32_t, uint32_t, uint32_t key, uint32_t state) noexcept {
            auto& self = *reinterpret_cast<Keyboard *>(data);
            const auto xkb_key = key + XKB_EVDEV_OFFSET;
            
            if (self._focus && self._state) {
                switch (state) {
                case WL_KEYBOARD_KEY_STATE_PRESSED: {
                    const xkb_keysym_t *syms;
                    const auto num_syms = xkb_state_key_get_syms(self._state.get(), xkb_key, &syms);

                    const auto shift = xkb_state_mod_name_is_active(self._state.get(), XKB_MOD_NAME_SHIFT, XKB_STATE_MODS_EFFECTIVE);
                    const auto ctrl = xkb_state_mod_name_is_active(self._state.get(), XKB_MOD_NAME_CTRL, XKB_STATE_MODS_EFFECTIVE);
                    const auto alt = xkb_state_mod_name_is_active(self._state.get(), XKB_MOD_NAME_ALT, XKB_STATE_MODS_EFFECTIVE);

                    for (auto i = 0; i < num_syms; ++i) {
                        self._focus->keysym(syms[i], shift, ctrl, alt);
                    }

                    const size_t chars = static_cast<size_t>(1 + xkb_state_key_get_utf8(self._state.get(), xkb_key, nullptr, 0));
                    if (chars > 0) {
                        const auto buf = std::make_unique_for_overwrite<char[]>(chars);
                        xkb_state_key_get_utf8(self._state.get(), xkb_key, buf.get(), chars);

                        self._focus->text(std::string_view(buf.get(), chars));
                    }
                    break;
                }
                case WL_KEYBOARD_KEY_STATE_RELEASED:
                default:
                    break;
                }
            }
        },
        .modifiers = [](void *data, wl_keyboard *, uint32_t, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) noexcept {
            auto& self = *reinterpret_cast<Keyboard *>(data);

            if (self._state) {
                xkb_state_update_mask(self._state.get(), mods_depressed, mods_latched, mods_locked, 0, 0, group);
            }
        },
        .repeat_info = [](void *, wl_keyboard *, int32_t, int32_t) noexcept {

        }
    };

    _keyboard.reset(wl_seat_get_keyboard(seat._seat.get()));
    wl_keyboard_add_listener(_keyboard.get(), &keyboard_listener, this);
}
