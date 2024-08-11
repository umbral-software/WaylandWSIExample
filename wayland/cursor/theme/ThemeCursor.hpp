#pragma once

#include "../CursorBase.hpp"

#include "wayland/WaylandPointer.hpp"

class ThemeCursor final : public CursorBase {
public:
    explicit ThemeCursor(wl_pointer *pointer, wl_surface *surface, wl_cursor *cursor);

    virtual void set_pointer(uint32_t serial) override;

private:
    wl_pointer *_pointer;

    WaylandPointer<wl_surface> _surface;
    int32_t _x, _y;
};
