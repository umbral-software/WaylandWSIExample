#include "ShapeCursor.hpp"

ShapeCursor::ShapeCursor(wp_cursor_shape_device_v1 *cursor_shape_device)
    :_device(cursor_shape_device)
{}

void ShapeCursor::set_pointer(uint32_t serial) {
    wp_cursor_shape_device_v1_set_shape(_device.get(), serial, WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DEFAULT);
}
