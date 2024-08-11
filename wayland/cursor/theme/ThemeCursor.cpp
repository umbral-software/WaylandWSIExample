#include "ThemeCursor.hpp"

// Like many clients, this class does not implement support for animated cursors
ThemeCursor::ThemeCursor(wl_pointer *pointer, wl_surface *surface, wl_cursor *cursor)
    :_pointer(pointer)
    ,_surface(surface)
{
    auto *image = cursor->images[0];
    wl_surface_attach(_surface.get(), wl_cursor_image_get_buffer(image), image->width, image->height);
    wl_surface_commit(_surface.get());

    _x = image->hotspot_x;
    _y = image->hotspot_y;
}

void ThemeCursor::set_pointer(uint32_t serial) {
    wl_pointer_set_cursor(_pointer, serial, _surface.get(), _x, _y);
}
