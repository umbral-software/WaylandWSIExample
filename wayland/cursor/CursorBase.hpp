#pragma once

#include "CursorType.hpp"

#include <cstdint>

class CursorBase {
protected:
    CursorBase() = default;
    CursorBase(const CursorBase&) = delete;
    CursorBase(CursorBase&&) noexcept = delete;

    CursorBase& operator=(CursorBase&) = delete;
    CursorBase& operator=(CursorBase&&) noexcept = delete;

public:
    virtual ~CursorBase();

    virtual void set_cursor_type(CursorType type) = 0;
    virtual void enter(uint32_t serial) = 0;
    virtual void leave() = 0;
};
