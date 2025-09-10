#include "ThemeCursor.hpp"
#include <chrono>
#include <condition_variable>
#include <mutex>

ThemeCursor::ThemeCursor(wl_cursor *cursor, wl_pointer *pointer, wl_surface *surface)
    :_cursor(cursor)
    ,_pointer(pointer)
    ,_surface(surface)
{}

ThemeCursor::~ThemeCursor() {
    kill_thread();
}

void ThemeCursor::set_pointer(uint32_t serial) {
    auto *image = _cursor->images[0];
    attach_buffer(nullptr, image);
    wl_pointer_set_cursor(
        _pointer, serial, _surface.get(),
        static_cast<int32_t>(image->hotspot_x),
        static_cast<int32_t>(image->hotspot_y)
    );

    if (_cursor->image_count > 1) {
        _thread_is_cancelled = false;
        _thread = std::thread(&ThemeCursor::thread_entry, this);
    }
}

void ThemeCursor::unset_pointer(uint32_t) {
    kill_thread();
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

    wl_surface_attach(
        _surface.get(),
        wl_cursor_image_get_buffer(image),
        static_cast<int32_t>(x_offset),
        static_cast<int32_t>(y_offset)
    );
    wl_surface_damage_buffer(
        _surface.get(),
        0, 0,
        static_cast<int32_t>(image->width),
        static_cast<int32_t>(image->height)
    );
    wl_surface_commit(_surface.get());
}

void ThemeCursor::kill_thread() {
    if (_thread.joinable()) {
        {
            const auto lock = std::unique_lock{ _mutex };
            _thread_is_cancelled = true;
            _cv.notify_all();
        }
        _thread.join();
    }
}

void ThemeCursor::thread_entry() noexcept {
    uint32_t image_index = 0;
    auto lock = std::unique_lock{_mutex };
    while (true) {
        const auto old_image = _cursor->images[image_index];

        if (_cv.wait_for(lock, std::chrono::milliseconds(old_image->delay), [this](){
            return _thread_is_cancelled;
        })) {
            // Thread is cancelled!
            return;
        }

        image_index = (image_index + 1) % _cursor->image_count;
        auto *image = _cursor->images[image_index];

        attach_buffer(old_image, image);
    }
}
