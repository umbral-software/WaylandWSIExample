#pragma once

#include <string>

class EventBase {
public:
    virtual ~EventBase() = default;

    virtual std::string to_string() const  = 0;
};
