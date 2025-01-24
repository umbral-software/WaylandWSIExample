#pragma once

#include "EventBase.hpp"
#include <xkbcommon/xkbcommon.h>

#include <string>

enum class KeyboardEventType {
    Keysym,
    Text,
    Modifiers
};

class KeyboardEventBase : public EventBase {
public:
    virtual KeyboardEventType type() const noexcept = 0;
};

class KeysymKeyboardEvent final : public KeyboardEventBase {
public:
    KeysymKeyboardEvent(xkb_keysym_t keysym, bool down)
        :_keysym(keysym)
        ,_down(down)
    {}
    KeysymKeyboardEvent(const KeysymKeyboardEvent&) = default;
    KeysymKeyboardEvent(KeysymKeyboardEvent&&) noexcept = default;
    ~KeysymKeyboardEvent() final = default;

    KeysymKeyboardEvent& operator=(const KeysymKeyboardEvent&) = default;
    KeysymKeyboardEvent& operator=(KeysymKeyboardEvent&&) = default;

    KeyboardEventType type() const noexcept final;

private:
    xkb_keysym_t _keysym;
    bool _down;
};

class TextKeyboardEvent final : public KeyboardEventBase {
public:
    TextKeyboardEvent(const char *buf, size_t size)
        :_text(buf, size)
    {}
    TextKeyboardEvent(const TextKeyboardEvent&) = default;
    TextKeyboardEvent(TextKeyboardEvent&&) noexcept = default;
    ~TextKeyboardEvent() final = default;

    TextKeyboardEvent& operator=(const TextKeyboardEvent&) = default;
    TextKeyboardEvent& operator=(TextKeyboardEvent&&) = default;

    KeyboardEventType type() const noexcept final;

private:
    std::string _text;
};

class ModifiersKeyboardEvent final : public KeyboardEventBase {
public:
    ModifiersKeyboardEvent(bool ctrl, bool shift, bool alt)
        :_ctrl(ctrl)
        ,_shift(shift)
        ,_alt(alt)
    {}
    ModifiersKeyboardEvent(const ModifiersKeyboardEvent&) = default;
    ModifiersKeyboardEvent(ModifiersKeyboardEvent&&) noexcept = default;
    ~ModifiersKeyboardEvent() final = default;

    ModifiersKeyboardEvent& operator=(const ModifiersKeyboardEvent&) = default;
    ModifiersKeyboardEvent& operator=(ModifiersKeyboardEvent&&) = default;

    KeyboardEventType type() const noexcept final;

private:
    bool _ctrl, _shift, _alt;
};
