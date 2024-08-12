#pragma once

#include <wayland-client.h>

class CursorBase {
public:
    virtual ~CursorBase();

    virtual void set_pointer(uint32_t serial) = 0;
    virtual void unset_pointer(uint32_t serial) = 0;
};
