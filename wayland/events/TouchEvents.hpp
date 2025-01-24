#pragma once

#include "EventBase.hpp"

#include <cstdint>
#include <utility>

enum class TouchEventType {
    Down,
    Up,
    Motion,
    Shape,
    Orientation
};

class TouchEventBase : public EventBase {
public:
    virtual TouchEventType type() const noexcept = 0;
};

class DownTouchEvent final : public TouchEventBase {
public:
    DownTouchEvent(uint32_t serial, uint32_t time, int x, int y)
        :_serial(serial)
        ,_time(time)
        ,_pos(x, y)
    {}
    DownTouchEvent(const DownTouchEvent&) = default;
    DownTouchEvent(DownTouchEvent&&) noexcept = default;
    ~DownTouchEvent() final = default;

    DownTouchEvent& operator=(const DownTouchEvent&) = default;
    DownTouchEvent& operator=(DownTouchEvent&&) = default;

    TouchEventType type() const noexcept final;

private:
    uint32_t _serial;
    uint32_t _time;
    std::pair<int, int> _pos;
};

class UpTouchEvent final : public TouchEventBase {
public:
    UpTouchEvent(uint32_t serial, uint32_t time)
        :_serial(serial)
        ,_time(time)
    {}
    UpTouchEvent(const UpTouchEvent&) = default;
    UpTouchEvent(UpTouchEvent&&) noexcept = default;
    ~UpTouchEvent() final = default;

    UpTouchEvent& operator=(const UpTouchEvent&) = default;
    UpTouchEvent& operator=(UpTouchEvent&&) = default;

    TouchEventType type() const noexcept final;

private:
    uint32_t _serial;
    uint32_t _time;
};

class MotionTouchEvent final : public TouchEventBase {
public:
    MotionTouchEvent(uint32_t time, int x, int y)
        :_pos(x, y)
    {}

    MotionTouchEvent(const MotionTouchEvent&) = default;
    MotionTouchEvent(MotionTouchEvent&&) noexcept = default;
    ~MotionTouchEvent() final = default;

    MotionTouchEvent& operator=(const MotionTouchEvent&) = default;
    MotionTouchEvent& operator=(MotionTouchEvent&&) = default;

    TouchEventType type() const noexcept final;

private:
    std::pair<int, int> _pos;
};

class ShapeTouchEvent final : public TouchEventBase {
public:
    ShapeTouchEvent(int major, int minor)
        :_size(major, minor)
    {}

    ShapeTouchEvent(const ShapeTouchEvent&) = default;
    ShapeTouchEvent(ShapeTouchEvent&&) noexcept = default;
    ~ShapeTouchEvent() final = default;

    ShapeTouchEvent& operator=(const ShapeTouchEvent&) = default;
    ShapeTouchEvent& operator=(ShapeTouchEvent&&) = default;

    TouchEventType type() const noexcept final;

private:
    std::pair<int, int> _size;
};

class OrientationTouchEvent final : public TouchEventBase {
public:
    OrientationTouchEvent(int angle)
        :_angle(angle)
    {}

    OrientationTouchEvent(const OrientationTouchEvent&) = default;
    OrientationTouchEvent(OrientationTouchEvent&&) noexcept = default;
    ~OrientationTouchEvent() final = default;

    OrientationTouchEvent& operator=(const OrientationTouchEvent&) = default;
    OrientationTouchEvent& operator=(OrientationTouchEvent&&) = default;

    TouchEventType type() const noexcept final;

private:
    int _angle;
};

