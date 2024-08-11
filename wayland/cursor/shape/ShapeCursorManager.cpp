#include "ShapeCursorManager.hpp"
#include "ShapeCursor.hpp"

ShapeCursorManager::ShapeCursorManager(wp_cursor_shape_manager_v1 *cursor_shape_manager)
    :_manager(cursor_shape_manager)
{}

std::unique_ptr<CursorBase> ShapeCursorManager::get_cursor(wl_pointer *pointer) {
    return std::make_unique<ShapeCursor>(wp_cursor_shape_manager_v1_get_pointer(_manager.get(), pointer));
}
