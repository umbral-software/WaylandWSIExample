#include "ShapeCursor.hpp"

static wp_cursor_shape_device_v1_shape cursor_type_to_shape(CursorType type) {
    switch (type) {
    case CursorType::Default:
        return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DEFAULT;
    case CursorType::Text:
        return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_TEXT;
    case CursorType::NSResize:
        return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NS_RESIZE;
    case CursorType::EWResize:
        return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_EW_RESIZE;
    case CursorType::NESWResize:
        return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NESW_RESIZE;
    case CursorType::NWSEResize:
        return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NWSE_RESIZE;
    default:
        return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DEFAULT;
    }
}

ShapeCursor::ShapeCursor(wp_cursor_shape_device_v1 *cursor_shape_device)
    :_device(cursor_shape_device)
{}

void ShapeCursor::set_cursor_type(CursorType type) {
    _shape = cursor_type_to_shape(type);
    do_update();
}

void ShapeCursor::enter(uint32_t serial) {
    _serial = serial;
    _shape = WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DEFAULT;
    do_update();
}

void ShapeCursor::leave() {

}

void ShapeCursor::do_update() {
    wp_cursor_shape_device_v1_set_shape(_device.get(), _serial, _shape);
}