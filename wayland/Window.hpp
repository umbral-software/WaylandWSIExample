#pragma once

#include "events/KeyboardEvents.hpp"
#include "events/PointerEvents.hpp"
#include "events/TouchEvents.hpp"
#include "WaylandPointer.hpp"

#include <optional>
#include <vector>

class Display;
class EventBase;

class Window {
public:
    explicit Window(Display& display);
    Window(const Window&) = delete;
    Window(Window&&) noexcept = delete;
    ~Window() = default;

    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) noexcept = delete;

    virtual void reconfigure() noexcept = 0;

    virtual void keyboard_events(const std::vector<std::unique_ptr<KeyboardEventBase>>& events) noexcept = 0;
    virtual void pointer_events(const std::vector<std::unique_ptr<PointerEventBase>>& events) noexcept = 0;
    virtual void touch_events(int id, const std::vector<std::unique_ptr<TouchEventBase>>& events) noexcept = 0;

    bool should_close() const noexcept;

    wl_display *display() noexcept;
    wl_surface *surface() noexcept;

protected:
    // Numerator of a fraction with DEFAULT_SCALE_DPI as the denominator
    uint32_t buffer_scale() const noexcept;
    std::pair<uint32_t, uint32_t> buffer_size() const noexcept;
    std::pair<uint32_t, uint32_t> surface_size() const noexcept;

    void set_should_close() noexcept;
    void toggle_fullscreen() noexcept;

    static constexpr uint32_t DEFAULT_SCALE_DPI = 120;

private:
    Display& _display;

    WaylandPointer<wl_surface> _surface;
    WaylandPointer<xdg_surface> _wm_surface;
    WaylandPointer<xdg_toplevel> _toplevel;

    // Optional protocols
    WaylandPointer<wp_content_type_v1> _content_type;
    WaylandPointer<wp_fractional_scale_v1> _fractional_scale;
    WaylandPointer<wp_viewport> _viewport;
    WaylandPointer<zxdg_toplevel_decoration_v1> _toplevel_decoration;

    bool _closed, _fullscreen, _maximized, _has_server_decorations;
    int32_t _actual_integer_scale;
    std::optional<int32_t> _desired_integer_scale;
    uint32_t _actual_fractional_scale;
    std::optional<uint32_t> _desired_fractional_scale;

    std::pair<int32_t, int32_t> _actual_surface_bounds;
    std::optional<std::pair<int32_t, int32_t>> _desired_surface_bounds;

    std::pair<int32_t, int32_t> _actual_surface_size;
    std::optional<std::pair<int32_t, int32_t>> _desired_surface_size;
};
