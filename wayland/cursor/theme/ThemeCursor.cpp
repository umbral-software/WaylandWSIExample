#include "ThemeCursor.hpp"

ThemeCursor::ThemeCursor(wl_cursor *cursor, wl_pointer *pointer, wl_surface *surface)
    :_cursor(cursor)
    ,_pointer(pointer)
    ,_surface(surface)
{}

ThemeCursor::~ThemeCursor() {
    if (!_thread_status.test_and_set(std::memory_order_relaxed)) {
        _thread.join();
    }
}

void ThemeCursor::set_pointer(uint32_t serial) {
    _serial = serial;
    _thread_status.clear(std::memory_order_release);

    wl_pointer_set_cursor(_pointer, _serial, _surface.get(), _cursor->images[0]->hotspot_x, _cursor->images[0]->hotspot_y);
    if (_cursor->image_count > 0) {
        _thread = std::thread(&ThemeCursor::thread_entry, this);
    }
}

void ThemeCursor::unset_pointer(uint32_t serial) {
    _thread_status.test_and_set(std::memory_order_relaxed);
    _thread.join();
}

void ThemeCursor::thread_entry() noexcept {
    uint32_t image_index = 0;
    while (!_thread_status.test(std::memory_order_relaxed)) {
        const auto *old_image = _cursor->images[image_index];
        std::this_thread::sleep_for(std::chrono::milliseconds(old_image->delay));

        image_index = (image_index + 1) % _cursor->image_count;
        auto *image = _cursor->images[image_index];

        wl_surface_attach(_surface.get(), wl_cursor_image_get_buffer(image), old_image->hotspot_x - image->hotspot_x, old_image->hotspot_y - image->hotspot_y);
        wl_surface_damage_buffer(_surface.get(), 0, 0, image->width, image->height);
        wl_surface_commit(_surface.get());
    }
}
