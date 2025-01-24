#include "TouchEvents.hpp"

TouchEventType DownTouchEvent::type() const noexcept {
    return TouchEventType::Down;
}

TouchEventType UpTouchEvent::type() const noexcept {
    return TouchEventType::Up;
}

TouchEventType MotionTouchEvent::type() const noexcept {
    return TouchEventType::Motion;
}

TouchEventType ShapeTouchEvent::type() const noexcept {
    return TouchEventType::Shape;
}

TouchEventType OrientationTouchEvent::type() const noexcept {
    return TouchEventType::Orientation;
}
