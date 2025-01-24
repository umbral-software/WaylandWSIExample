#pragma once

#include "events/TouchEvents.hpp"

#include <memory>
#include <vector>

class Window;

class TouchPoint {
    friend class Touch;
public:
    TouchPoint(int id, Window& focus);

    template<typename T, typename... Args>
    void add_event(Args... args) {
        _events.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

private:
    void clear_events();
    void send_events();

private:
    int _id;
    Window * _focus;
    std::vector<std::unique_ptr<TouchEventBase>> _events;
};
