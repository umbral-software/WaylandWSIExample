#pragma once

#include "../CursorBase.hpp"

#include "wayland/WaylandPointer.hpp"

class ShapeCursor final : public CursorBase {
public:
    explicit ShapeCursor(wp_cursor_shape_device_v1 *cursor_shape_device);

    virtual void set_pointer(uint32_t serial) override;

private:
    WaylandPointer<wp_cursor_shape_device_v1> _device;
};
