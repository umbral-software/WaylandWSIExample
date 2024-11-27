#include "ThemeCursorManager.hpp"
#include "ThemeCursor.hpp"

static constexpr int DEFAULT_CURSOR_SIZE = 24;

static int get_cursor_size() {
    const auto xcursor_size_str = getenv("XCURSOR_SIZE");
    if (!xcursor_size_str) {
        return DEFAULT_CURSOR_SIZE;
    }

    const auto cursor_size = atoi(xcursor_size_str);
    if (!cursor_size) {
        return DEFAULT_CURSOR_SIZE;
    }

    return cursor_size;
}

ThemeCursorManager::ThemeCursorManager(wl_compositor *compositor, wl_shm *shm)
    :_compositor(compositor)
    ,_theme(wl_cursor_theme_load(nullptr, get_cursor_size(), shm))
{

}

std::unique_ptr<CursorBase> ThemeCursorManager::get_cursor(wl_pointer *pointer) {
    return std::make_unique<ThemeCursor>(_theme.get(), pointer, wl_compositor_create_surface(_compositor));
}

