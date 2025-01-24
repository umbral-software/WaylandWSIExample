#include "TouchEvents.hpp"

DownTouchEvent::DownTouchEvent(uint32_t serial, uint32_t time, int x, int y)
    :_serial(serial)
    ,_time(time)
    ,_pos(x, y)
{}

TouchEventType DownTouchEvent::type() const noexcept {
    return TouchEventType::Down;
}

UpTouchEvent::UpTouchEvent(uint32_t serial, uint32_t time)
    :_serial(serial)
    ,_time(time)
{}

TouchEventType UpTouchEvent::type() const noexcept {
    return TouchEventType::Up;
}

MotionTouchEvent::MotionTouchEvent(uint32_t time, int x, int y)
    :_time(time)
    ,_pos(x, y)
{}

TouchEventType MotionTouchEvent::type() const noexcept {
    return TouchEventType::Motion;
}

ShapeTouchEvent::ShapeTouchEvent(int major, int minor)
    :_size(major, minor)
{}

TouchEventType ShapeTouchEvent::type() const noexcept {
    return TouchEventType::Shape;
}

OrientationTouchEvent::OrientationTouchEvent(int angle)
    :_angle(angle)
{}

TouchEventType OrientationTouchEvent::type() const noexcept {
    return TouchEventType::Orientation;
}
