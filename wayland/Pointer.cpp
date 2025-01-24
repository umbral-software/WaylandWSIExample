#include "Pointer.hpp"

#include "Display.hpp"
#include "Seat.hpp"
#include "Window.hpp"

#include <sstream>

static constexpr uint32_t LINUX_MOUSE_INPUT_CODE_OFFSET = 0x110;

class EnterPointerEvent final : public EventBase {
public:
    EnterPointerEvent(uint32_t serial, int x, int y)
        :_serial(serial)
        ,_pos(x, y)
    {}
    EnterPointerEvent(const EnterPointerEvent&) = default;
    EnterPointerEvent(EnterPointerEvent&&) noexcept = default;
    ~EnterPointerEvent() final = default;

    EnterPointerEvent& operator=(const EnterPointerEvent&) = default;
    EnterPointerEvent& operator=(EnterPointerEvent&&) = default;

    std::string to_string() const final {
        std::stringstream ss;
        ss << "Enter (serial: " << _serial << ", pos: (" << _pos.first << ", " << _pos.second  << "))"; 
        return ss.str();
    }

private:
    uint32_t _serial;
    std::pair<int, int> _pos;
};

class LeavePointerEvent final : public EventBase {
public:
    LeavePointerEvent(uint32_t serial)
        :_serial(serial)
    {}
    LeavePointerEvent(const LeavePointerEvent&) = default;
    LeavePointerEvent(LeavePointerEvent&&) noexcept = default;
    ~LeavePointerEvent() final = default;

    LeavePointerEvent& operator=(const LeavePointerEvent&) = default;
    LeavePointerEvent& operator=(LeavePointerEvent&&) = default;

    std::string to_string() const final {
        std::stringstream ss;
        ss << "Leave (serial: " << _serial  << ")"; 
        return ss.str();
    }

private:
    uint32_t _serial;
};

class MotionPointerEvent final : public EventBase {
public:
    MotionPointerEvent(uint32_t serial, int x, int y)
        :_serial(serial)
        ,_pos(x, y)
    {}
    MotionPointerEvent(const MotionPointerEvent&) = default;
    MotionPointerEvent(MotionPointerEvent&&) noexcept = default;
    ~MotionPointerEvent() final = default;

    MotionPointerEvent& operator=(const MotionPointerEvent&) = default;
    MotionPointerEvent& operator=(MotionPointerEvent&&) = default;

    std::string to_string() const final {
        std::stringstream ss;
        ss << "Motion (serial: " << _serial << ", pos: (" << _pos.first << ", " << _pos.second  << "))"; 
        return ss.str();
    }

private:
    uint32_t _serial;
    std::pair<int, int> _pos;
};

class ButtonPointerEvent final : public EventBase {
public:
    ButtonPointerEvent(uint32_t serial, uint32_t time, uint32_t button, bool pressed)
        :_serial(serial)
        ,_time(time)
        ,_button(button)
        ,_pressed(pressed)
    {}
    ButtonPointerEvent(const ButtonPointerEvent&) = default;
    ButtonPointerEvent(ButtonPointerEvent&&) noexcept = default;
    ~ButtonPointerEvent() final = default;

    ButtonPointerEvent& operator=(const ButtonPointerEvent&) = default;
    ButtonPointerEvent& operator=(ButtonPointerEvent&&) = default;

    std::string to_string() const final {
        std::stringstream ss;
        ss << "Button (serial: " << _serial << ", time: " << _time << ", button: " << _button << ", pressed: " << _pressed << ")"; 
        return ss.str();
    }

private:
    uint32_t _serial;
    uint32_t _time;
    uint32_t _button;
    bool _pressed;
};

class AxisPointerEvent final : public EventBase {
public:
    AxisPointerEvent(uint32_t time, int32_t value, bool horizontal)
        :_time(time)
        ,_value(value)
        ,_horizontal(horizontal)
    {}
    AxisPointerEvent(const AxisPointerEvent&) = default;
    AxisPointerEvent(AxisPointerEvent&&) noexcept = default;
    ~AxisPointerEvent() final = default;

    AxisPointerEvent& operator=(const AxisPointerEvent&) = default;
    AxisPointerEvent& operator=(AxisPointerEvent&&) = default;

    std::string to_string() const final {
        std::stringstream ss;
        ss << "Axis (time: " << ", value: " << _value << ", horizontal: " << _horizontal << ")"; 
        return ss.str();
    }

private:
    uint32_t _time;
    int32_t _value;
    bool _horizontal;
};

Pointer::Pointer(Seat& seat)
    :_display(seat._display)
    ,_focus(nullptr)
{
    static constexpr wl_pointer_listener pointer_listener {
        .enter = [](void *data, wl_pointer *, uint32_t serial, wl_surface *surface, wl_fixed_t x, wl_fixed_t y) noexcept {
            auto& self = *static_cast<Pointer *>(data);

            if (surface) {
                self._focus = static_cast<Window *>(wl_surface_get_user_data(surface));
                self._cursor->set_pointer(serial);
                self._events.emplace_back(std::make_unique<EnterPointerEvent>(serial, wl_fixed_to_int(x), wl_fixed_to_int(y)));
            }
        },
        .leave = [](void *data, wl_pointer *, uint32_t serial, wl_surface *) noexcept {
            auto& self = *static_cast<Pointer *>(data);

            self._events.emplace_back(std::make_unique<LeavePointerEvent>(serial));

            if (self._focus && !self._events.empty()) {
                self._focus->pointer_events(self._events);
            }

            self._events.clear();
            self._cursor->unset_pointer(serial);
            self._focus = nullptr;
        },
        .motion  = [](void *data, wl_pointer *, uint32_t serial, wl_fixed_t x, wl_fixed_t y) noexcept {
            auto& self = *static_cast<Pointer *>(data);

            self._events.emplace_back(std::make_unique<MotionPointerEvent>(serial, wl_fixed_to_int(x), wl_fixed_to_int(y)));
        },
        .button = [](void *data, wl_pointer *, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) noexcept {
            auto& self = *static_cast<Pointer *>(data);

            switch (state) {
            case WL_POINTER_BUTTON_STATE_RELEASED:
                self._events.emplace_back(std::make_unique<ButtonPointerEvent>(serial, time, button - LINUX_MOUSE_INPUT_CODE_OFFSET, false));
                break;
            case WL_POINTER_BUTTON_STATE_PRESSED:
                self._events.emplace_back(std::make_unique<ButtonPointerEvent>(serial, time, button - LINUX_MOUSE_INPUT_CODE_OFFSET, true));
                break;
            default:
                break;
            }            
        },
        .axis = [](void *data, wl_pointer *, uint32_t time, uint32_t axis, wl_fixed_t value) noexcept {
            auto& self = *static_cast<Pointer *>(data);

            switch (axis) {
            case WL_POINTER_AXIS_VERTICAL_SCROLL:
                self._events.emplace_back(std::make_unique<AxisPointerEvent>(time, wl_fixed_to_int(value), false));
                break;
            case WL_POINTER_AXIS_HORIZONTAL_SCROLL:
                self._events.emplace_back(std::make_unique<AxisPointerEvent>(time, wl_fixed_to_int(value), true));
                break;
            default:
                break;
            }
        },
        .frame = [](void *data, wl_pointer *) noexcept {
            auto& self = *static_cast<Pointer *>(data);

            if (self._focus && !self._events.empty()) {
                self._focus->pointer_events(self._events);
            }
            self._events.clear();
        },
        // The version of wl_seat (7) used will return these 3 events
        // Handling them is left as an exercise for the reader, but note that
        //   axis_discrete was deprecated in v8 in favor of axis_value120
        .axis_source = [](void *, wl_pointer *, uint32_t) noexcept {},
        .axis_stop = [](void *, wl_pointer *, uint32_t, uint32_t) noexcept {},
        .axis_discrete = [](void *, wl_pointer *, uint32_t, int32_t) noexcept {},
        .axis_value120 = [](void *, wl_pointer *, uint32_t, int32_t) noexcept {},
        .axis_relative_direction = [](void *, wl_pointer *, uint32_t, uint32_t) noexcept {},
    };

    _pointer.reset(wl_seat_get_pointer(seat._seat.get()));
    wl_pointer_add_listener(_pointer.get(), &pointer_listener, this);

    _cursor = _display._cursor_manager->get_cursor(_pointer.get());
}
