#include "ThemeCursor.hpp"

ThemeCursor::ThemeCursor(wl_cursor *cursor, wl_pointer *pointer, wl_surface *surface)
    :_cursor(cursor)
    ,_pointer(pointer)
    ,_surface(surface)
{}

ThemeCursor::~ThemeCursor() {
    if (_thread.joinable()) {
        _thread_status.test_and_set(std::memory_order_relaxed);
        _thread.join();
    }
}

void ThemeCursor::set_pointer(uint32_t serial) {
    auto *image = _cursor->images[0];
    attach_buffer(nullptr, image);
    wl_pointer_set_cursor(_pointer, serial, _surface.get(), image->hotspot_x, image->hotspot_y);

    if (_cursor->image_count > 1) {
        _thread_status.clear(std::memory_order_relaxed);
        _thread = std::thread(&ThemeCursor::thread_entry, this);
    }
}

void ThemeCursor::unset_pointer(uint32_t serial) {
    if (_thread.joinable()) {
        _thread_status.test_and_set(std::memory_order_relaxed);
        _thread.join();
    }
}

void ThemeCursor::attach_buffer(wl_cursor_image *old_image, wl_cursor_image *image) {
    uint32_t x_offset, y_offset;
    if (old_image) {
        x_offset = old_image->hotspot_x - image->hotspot_x;
        y_offset = old_image->hotspot_y - image->hotspot_y;
    } else {
        x_offset = 0;
        y_offset = 0;
    }

    wl_surface_attach(_surface.get(), wl_cursor_image_get_buffer(image), x_offset, y_offset);
    wl_surface_damage_buffer(_surface.get(), 0, 0, image->width, image->height);
    wl_surface_commit(_surface.get());
}

void ThemeCursor::thread_entry() noexcept {
    uint32_t image_index = 0;
    while (!_thread_status.test(std::memory_order_relaxed)) {
        const auto old_image = _cursor->images[image_index];

        image_index = (image_index + 1) % _cursor->image_count;
        auto *image = _cursor->images[image_index];

        attach_buffer(old_image, image);
        std::this_thread::sleep_for(std::chrono::milliseconds(image->delay));
    }
}
