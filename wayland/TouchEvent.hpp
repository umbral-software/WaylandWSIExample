#pragma once

#include <string>

class TouchEvent {
public:
    virtual ~TouchEvent() = default;

    virtual std::string to_string() const  = 0;
};
