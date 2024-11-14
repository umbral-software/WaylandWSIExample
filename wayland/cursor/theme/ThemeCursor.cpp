#include "ThemeCursor.hpp"

static const char *cursor_type_to_name(CursorType type) {
    switch (type) {
    case CursorType::Default:
        return "default";
    case CursorType::Text:
        return "text";
    case CursorType::NSResize:
        return "ns-resize";
    case CursorType::EWResize:
        return "ew-resize";
    case CursorType::NESWResize:
        return "nesw-resize";
    case CursorType::NWSEResize:
        return "nwse-resize";
    default:
        return "default";
    }
}

ThemeCursor::ThemeCursor(wl_cursor_theme *theme, wl_pointer *pointer, wl_surface *surface)
    :_theme(theme)
    ,_pointer(pointer)
    ,_surface(surface)
    ,_cursor(wl_cursor_theme_get_cursor(_theme, cursor_type_to_name(CursorType::Default)))
{}

ThemeCursor::~ThemeCursor() {
    stop_thread();
}

void ThemeCursor::set_cursor_type(CursorType type) {
    _cursor = wl_cursor_theme_get_cursor(_theme, cursor_type_to_name(type));
    do_update();
}

void ThemeCursor::enter(uint32_t serial) {
    _serial = serial;
    do_update();
}

void ThemeCursor::leave() {
    stop_thread();
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

void ThemeCursor::thread_entry() noexcept {
    uint32_t image_index = 0;
    while (!_thread_status.test(std::memory_order_relaxed)) {
        const auto old_image = _cursor->images[image_index];
        std::this_thread::sleep_for(std::chrono::milliseconds(old_image->delay));

        image_index = (image_index + 1) % _cursor->image_count;
        auto *image = _cursor->images[image_index];

        attach_buffer(old_image, image);
    }
}

void ThemeCursor::do_update() {
    stop_thread();

    auto *image = _cursor->images[0];
    attach_buffer(nullptr, image);
    wl_pointer_set_cursor(
        _pointer, _serial, _surface.get(),
        static_cast<int32_t>(image->hotspot_x),
        static_cast<int32_t>(image->hotspot_y)
    );

    if (_cursor->image_count > 1) {
        start_thread();
    }
}

void ThemeCursor::start_thread() {
    _thread_status.clear(std::memory_order_relaxed);
    _thread = std::thread(&ThemeCursor::thread_entry, this);
}

void ThemeCursor::stop_thread() {
    if (_thread.joinable()) {
        _thread_status.test_and_set(std::memory_order_relaxed);
        _thread.join();
    }
}