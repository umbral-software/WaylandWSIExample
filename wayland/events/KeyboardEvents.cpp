#include "KeyboardEvents.hpp"

KeysymKeyboardEvent::KeysymKeyboardEvent(xkb_keysym_t keysym, bool down)
    :_keysym(keysym)
    ,_down(down)
{}

xkb_keysym_t KeysymKeyboardEvent::keysym() const noexcept {
    return _keysym;
}

KeyboardEventType KeysymKeyboardEvent::type() const noexcept {
    return KeyboardEventType::Keysym;
}

bool KeysymKeyboardEvent::is_down() const noexcept {
    return _down;
}

TextKeyboardEvent::TextKeyboardEvent(const char *buf, size_t size)
    :_text(buf, size)
{}

const char *TextKeyboardEvent::text() const noexcept {
    return _text.c_str();
}

KeyboardEventType TextKeyboardEvent::type() const noexcept {
    return KeyboardEventType::Text;
}

ModifiersKeyboardEvent::ModifiersKeyboardEvent(bool shift, bool ctrl, bool alt)
    :_shift(shift)
    ,_ctrl(ctrl)
    ,_alt(alt)
{}

bool ModifiersKeyboardEvent::ctrl() const noexcept {
    return _ctrl;
}

bool ModifiersKeyboardEvent::shift() const noexcept {
    return _shift;
}

bool ModifiersKeyboardEvent::alt() const noexcept {
    return _alt;
}

KeyboardEventType ModifiersKeyboardEvent::type() const noexcept {
    return KeyboardEventType::Modifiers;
}

