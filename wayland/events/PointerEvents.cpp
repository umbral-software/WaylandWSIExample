#include "PointerEvents.hpp"

PointerEventType EnterPointerEvent::type() const noexcept {
    return PointerEventType::Enter;
}

PointerEventType LeavePointerEvent::type() const noexcept {
    return PointerEventType::Leave;
}

PointerEventType MotionPointerEvent::type() const noexcept {
    return PointerEventType::Motion;
}

PointerEventType ButtonPointerEvent::type() const noexcept {
    return PointerEventType::Button;
}

PointerEventType AxisPointerEvent::type() const noexcept {
    return PointerEventType::Axis;
}
