#pragma once

#include <xkbcommon/xkbcommon.h>

#include <memory>

struct XkbDeleter {
    void operator()(xkb_context *context) const noexcept {
        xkb_context_unref(context);
    }

    void operator()(xkb_keymap *keymap) const noexcept {
        xkb_keymap_unref(keymap);
    }
    
    void operator()(xkb_state *state) const noexcept {
        xkb_state_unref(state);
    }
};

template<typename T>
concept XkbDeletable = requires(T *t, const XkbDeleter& d) {
    requires noexcept(d(t));
};

template<XkbDeletable T>
using XkbPointer = std::unique_ptr<T, XkbDeleter>;
