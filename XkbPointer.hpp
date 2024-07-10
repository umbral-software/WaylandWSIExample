#pragma once

#include <xkbcommon/xkbcommon.h>

#include <memory>

struct XkbDeleter {
    void operator()(void *ptr) const noexcept {
        free(ptr);
    }
};

template<typename T>
concept XkbDeletable = requires(T *t, const XkbDeleter& d) {
    requires noexcept(d(t));
};

template<XkbDeletable T>
using XkbPointer = std::unique_ptr<T, XkbDeleter>;
