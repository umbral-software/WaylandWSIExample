#pragma once

#include "../CursorBase.hpp"

#include "wayland/WaylandPointer.hpp"

#include <thread>

class ThemeCursor final : public CursorBase {
public:
    explicit ThemeCursor(wl_cursor_theme *cursor, wl_pointer *pointer, wl_surface *surface);
    ThemeCursor(const ThemeCursor&) = delete;
    ThemeCursor(ThemeCursor&&) noexcept = delete;
    ~ThemeCursor();

    ThemeCursor& operator=(const ThemeCursor&) = delete;
    ThemeCursor& operator=(ThemeCursor&&) noexcept = delete;

    virtual void set_cursor_type(CursorType type) override;
    virtual void enter(uint32_t serial) override;
    virtual void leave() override;

private:
    void attach_buffer(wl_cursor_image *old_image, wl_cursor_image *image);
    void do_update();
    void start_thread();
    void stop_thread();
    void thread_entry() noexcept;

private:
    wl_cursor_theme *_theme;
    wl_pointer *_pointer;
    WaylandPointer<wl_surface> _surface;

    wl_cursor *_cursor;
    uint32_t _serial;

    std::atomic_flag _thread_status;
    std::thread _thread;
};
