#include "Touch.hpp"

#include "Display.hpp"
#include "Seat.hpp"
#include "Window.hpp"
#include "TouchPoint.hpp"

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

            self._touchpoints.clear();
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
