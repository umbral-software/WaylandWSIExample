#include "KeyboardEvents.hpp"

KeyboardEventType KeysymKeyboardEvent::type() const noexcept {
    return KeyboardEventType::Keysym;
}

KeyboardEventType ModifiersKeyboardEvent::type() const noexcept {
    return KeyboardEventType::Modifiers;
}

KeyboardEventType TextKeyboardEvent::type() const noexcept {
    return KeyboardEventType::Text;
}
