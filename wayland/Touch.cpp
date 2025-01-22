#include "Touch.hpp"

#include "Display.hpp"
#include "Window.hpp"
#include "Seat.hpp"
#include "wayland/TouchEvent.hpp"
#include "wayland/TouchPoint.hpp"
#include <sstream>

class DownTouchEvent final : public TouchEvent {
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

    std::string to_string() const final {
        std::stringstream ss;
        ss << "Down (serial: " << _serial << ", time: " << _time << ", pos: (" << _pos.first << ", " << _pos.second  << "))"; 
        return ss.str();
    }

private:
    uint32_t _serial;
    uint32_t _time;
    std::pair<int, int> _pos;
};

class MotionTouchEvent final : public TouchEvent {
public:
    MotionTouchEvent(uint32_t time, int x, int y)
        :_pos(x, y)
    {}

    MotionTouchEvent(const MotionTouchEvent&) = default;
    MotionTouchEvent(MotionTouchEvent&&) noexcept = default;
    ~MotionTouchEvent() final = default;

    MotionTouchEvent& operator=(const MotionTouchEvent&) = default;
    MotionTouchEvent& operator=(MotionTouchEvent&&) = default;

    std::string to_string() const final {
        std::stringstream ss;
        ss << "Motion (pos: (" << _pos.first << ", " << _pos.second  << "))"; 
        return ss.str();
    }

private:
    std::pair<int, int> _pos;
};

class OrientationTouchEvent final : public TouchEvent {
public:
    OrientationTouchEvent(int angle)
        :_angle(angle)
    {}

    OrientationTouchEvent(const OrientationTouchEvent&) = default;
    OrientationTouchEvent(OrientationTouchEvent&&) noexcept = default;
    ~OrientationTouchEvent() final = default;

    OrientationTouchEvent& operator=(const OrientationTouchEvent&) = default;
    OrientationTouchEvent& operator=(OrientationTouchEvent&&) = default;

    std::string to_string() const final {
        std::stringstream ss;
        ss << "Orientation (angle: " << _angle << "degrees))"; 
        return ss.str();
    }

private:
    int _angle;
};

class ShapeTouchEvent final : public TouchEvent {
public:
    ShapeTouchEvent(int major, int minor)
        :_size(major, minor)
    {}

    ShapeTouchEvent(const ShapeTouchEvent&) = default;
    ShapeTouchEvent(ShapeTouchEvent&&) noexcept = default;
    ~ShapeTouchEvent() final = default;

    ShapeTouchEvent& operator=(const ShapeTouchEvent&) = default;
    ShapeTouchEvent& operator=(ShapeTouchEvent&&) = default;

    std::string to_string() const final {
        std::stringstream ss;
        ss << "Shape (size: (" << _size.first << ", " << _size.second  << "))"; 
        return ss.str();
    }

private:
    std::pair<int, int> _size;
};

class UpTouchEvent final : public TouchEvent {
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

    std::string to_string() const final {
        std::stringstream ss;
        ss << "Up (serial: " << _serial << ", time: " << _time << "))"; 
        return ss.str();
    }

private:
    uint32_t _serial;
    uint32_t _time;
};

Touch::Touch(Seat& seat)
    :_display(seat._display)
{
    static constexpr wl_touch_listener touch_listener {
        .down = [](void *data, struct wl_touch *, uint32_t serial, uint32_t time, wl_surface *surface, int32_t id, wl_fixed_t x, wl_fixed_t y) {
            auto& self = *static_cast<Touch *>(data);
            auto& focus = *static_cast<Window *>(wl_surface_get_user_data(surface));

            auto [p, success] = self._touchpoints.emplace(id, TouchPoint{id, focus});
            if (success && p != self._touchpoints.end()) {
                p->second.add_event<DownTouchEvent>(serial, time, wl_fixed_to_int(x), wl_fixed_to_int(y));
            }
        },
        .up = [](void *data, struct wl_touch *, uint32_t serial, uint32_t time, int32_t id) {
            auto& self = *static_cast<Touch *>(data);
            auto p = self._touchpoints.find(id);

            if (p != self._touchpoints.end()) {
                p->second.add_event<UpTouchEvent>(serial, time);
                p->second.send_events();
                self._touchpoints.erase(p);
            }
        },
        .motion = [](void *data, struct wl_touch *, uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y) {
            auto& self = *static_cast<Touch *>(data);
            auto p = self._touchpoints.find(id);

            if (p != self._touchpoints.end()) {
                p->second.add_event<MotionTouchEvent>(time, wl_fixed_to_int(x), wl_fixed_to_int(y));
            }
        },
        .frame = [](void *data, struct wl_touch *) {
            auto& self = *static_cast<Touch *>(data);
            
            for (auto& [id, touchpoint] : self._touchpoints) {
                touchpoint.send_events();   
                touchpoint._events.clear();
            }
        },
        .cancel = [](void *data, struct wl_touch *) {
            auto& self = *static_cast<Touch *>(data);

            for (auto& [id, touchpoint] : self._touchpoints) {
                touchpoint._events.clear();
            }
        },
        .shape = [](void *data, struct wl_touch *, int32_t id, wl_fixed_t major, wl_fixed_t minor) {
            auto& self = *static_cast<Touch *>(data);
            auto p = self._touchpoints.find(id);

            if (p != self._touchpoints.end()) {
                p->second.add_event<ShapeTouchEvent>(wl_fixed_to_int(major), wl_fixed_to_int(minor));
            }            
        },
        .orientation = [](void *data, struct wl_touch *, int32_t id, wl_fixed_t orientation){
            auto& self = *static_cast<Touch *>(data);
            auto p = self._touchpoints.find(id);

            if (p != self._touchpoints.end()) {
                p->second.add_event<OrientationTouchEvent>(wl_fixed_to_int(orientation));
            }  
        }
    };
    _touch.reset(wl_seat_get_touch(seat._seat.get()));
    wl_touch_add_listener(_touch.get(), &touch_listener, this);
}
