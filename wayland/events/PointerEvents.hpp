#pragma once

#include "EventBase.hpp"

#include <cstdint>
#include <utility>

enum class PointerEventType {
    Enter,
    Leave,
    Motion,
    Button,
    Axis
};

class PointerEventBase : public EventBase {
public:
    virtual PointerEventType type() const noexcept = 0;
};

class EnterPointerEvent final : public PointerEventBase {
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

    PointerEventType type() const noexcept final;

private:
    uint32_t _serial;
    std::pair<int, int> _pos;
};

class LeavePointerEvent final : public PointerEventBase {
public:
    LeavePointerEvent(uint32_t serial)
        :_serial(serial)
    {}
    LeavePointerEvent(const LeavePointerEvent&) = default;
    LeavePointerEvent(LeavePointerEvent&&) noexcept = default;
    ~LeavePointerEvent() final = default;

    LeavePointerEvent& operator=(const LeavePointerEvent&) = default;
    LeavePointerEvent& operator=(LeavePointerEvent&&) = default;

    PointerEventType type() const noexcept final;

private:
    uint32_t _serial;
};

class MotionPointerEvent final : public PointerEventBase {
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

    PointerEventType type() const noexcept final;

private:
    uint32_t _serial;
    std::pair<int, int> _pos;
};

class ButtonPointerEvent final : public PointerEventBase {
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

    PointerEventType type() const noexcept final;

private:
    uint32_t _serial;
    uint32_t _time;
    uint32_t _button;
    bool _pressed;
};

class AxisPointerEvent final : public PointerEventBase {
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

    PointerEventType type() const noexcept final;

private:
    uint32_t _time;
    int32_t _value;
    bool _horizontal;
};
