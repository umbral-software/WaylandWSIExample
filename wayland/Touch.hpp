#pragma once

#include "TouchPoint.hpp"
#include "WaylandPointer.hpp"

#include <unordered_map>

class Display;
class Seat;

class Touch {
public:
    explicit Touch(Seat& seat);
    Touch(const Touch&) = delete;
    Touch(Touch&&) noexcept = delete;
    ~Touch() = default;

    Touch& operator=(const Touch&) = delete;
    Touch& operator=(Touch&&) noexcept = delete;

private:
    Display& _display;

    WaylandPointer<wl_touch> _touch;
    std::unordered_map<int32_t, TouchPoint> _touchpoints;
};
