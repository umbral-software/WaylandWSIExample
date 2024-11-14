#pragma once

#include "CursorType.hpp"

#include <cstdint>

class CursorBase {
public:
    virtual ~CursorBase();

    virtual void set_cursor_type(CursorType type) = 0;
    virtual void enter(uint32_t serial) = 0;
    virtual void leave() = 0;
};
