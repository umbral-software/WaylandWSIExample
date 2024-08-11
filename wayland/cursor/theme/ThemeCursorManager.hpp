#pragma once

#include "../CursorManagerBase.hpp"
#include "wayland/WaylandPointer.hpp"

class ThemeCursorManager final : public CursorManagerBase {
public:
    explicit ThemeCursorManager(wl_compositor *compositor, wl_shm *shm);

    virtual std::unique_ptr<CursorBase> get_cursor(wl_pointer *pointer) override;
private:
    wl_compositor *_compositor;

    WaylandPointer<wl_cursor_theme> _theme;
};
