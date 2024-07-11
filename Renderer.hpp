#pragma once

#include "RendererBase.hpp"

class Window;

class Renderer : private RendererBase {
public:
    Renderer(Window& window);

    FrameData& frame() noexcept;
    ImageData& image() noexcept;
    void render();

private:
    void rebuild_swapchain();

private:
    const Window& _window;
    
    size_t _frame_index;
    
    std::pair<uint32_t, uint32_t> _swapchain_size;
    uint32_t _image_index;
};
