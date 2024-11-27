#pragma once

#include "../CursorBase.hpp"

#include "wayland/WaylandPointer.hpp"

class ShapeCursor final : public CursorBase {
public:
    explicit ShapeCursor(wp_cursor_shape_device_v1 *cursor_shape_device);

    virtual void set_cursor_type(CursorType type) override;
    virtual void enter(uint32_t serial) override;
    virtual void leave() override;

private:
    void do_update();

private:
    WaylandPointer<wp_cursor_shape_device_v1> _device;
    wp_cursor_shape_device_v1_shape _shape;
    uint32_t _serial;
};
