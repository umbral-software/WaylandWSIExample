#pragma once

#include "RendererBase.hpp"

class Window;

class Renderer : private RendererBase {
public:
    Renderer(Window& window);
    ~Renderer();

    FrameData& frame() noexcept;
    void render();

private:
    void record_command_buffer();
private:
    const Window& _window;

    VkPhysicalDevice _physical_device;
    uint32_t _queue_family_index;
    
    VkQueue _queue;
    
    size_t _frame_index;
};
