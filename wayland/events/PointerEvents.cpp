#include "PointerEvents.hpp"
#include <cstdint>

EnterPointerEvent::EnterPointerEvent(uint32_t serial, int x, int y)
    :_serial(serial)
    ,_pos(x, y)
{}

PointerEventType EnterPointerEvent::type() const noexcept {
    return PointerEventType::Enter;
}

LeavePointerEvent::LeavePointerEvent(uint32_t serial)
    :_serial(serial)
{}

PointerEventType LeavePointerEvent::type() const noexcept {
    return PointerEventType::Leave;
}

MotionPointerEvent::MotionPointerEvent(uint32_t serial, int x, int y)
    :_serial(serial)
    ,_pos(x, y)
{}

const std::pair<int, int>& MotionPointerEvent::position() const noexcept {
    return _pos;
}

PointerEventType MotionPointerEvent::type() const noexcept {
    return PointerEventType::Motion;
}

ButtonPointerEvent::ButtonPointerEvent(uint32_t serial, uint32_t time, uint32_t button, bool down)
    :_serial(serial)
    ,_time(time)
    ,_button(button)
    ,_down(down)
{}

uint32_t ButtonPointerEvent::button_index() const noexcept {
    return _button;
}

bool ButtonPointerEvent::is_down() const noexcept {
    return _down;
}

PointerEventType ButtonPointerEvent::type() const noexcept {
    return PointerEventType::Button;
}

AxisPointerEvent::AxisPointerEvent(uint32_t time, int32_t value, bool horizontal)
    :_time(time)
    ,_value(value)
    ,_horizontal(horizontal)
{}

bool AxisPointerEvent::is_horizontal() const noexcept {
    return _horizontal;
}

bool AxisPointerEvent::is_vertical() const noexcept {
    return !_horizontal;
}

PointerEventType AxisPointerEvent::type() const noexcept {
    return PointerEventType::Axis;
}

int32_t AxisPointerEvent::value() const noexcept {
    return _value;
}
