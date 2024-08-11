#pragma once

#include "../CursorManagerBase.hpp"
#include "wayland/WaylandPointer.hpp"

class ShapeCursorManager final : public CursorManagerBase {
public:
    explicit ShapeCursorManager(wp_cursor_shape_manager_v1 *cursor_shape_manager);

    std::unique_ptr<CursorBase> get_cursor(wl_pointer *pointer) override;
private:
    WaylandPointer<wp_cursor_shape_manager_v1> _manager;
};
