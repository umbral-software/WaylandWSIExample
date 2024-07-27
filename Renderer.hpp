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

    VkPhysicalDevice _physical_device;
    uint32_t _queue_family_index;
    VkSurfaceFormatKHR _surface_format;
    VkFormat _depth_format;
    
    size_t _frame_index;
    
    VkExtent2D _swapchain_size;
    uint32_t _image_index;
    bool _rebuild_required;
};
