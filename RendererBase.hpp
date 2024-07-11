#pragma once

#include <vulkan/vulkan.h>

#include <array>
#include <vector>

static constexpr size_t NUM_FRAMES_IN_FLIGHT = 2;

struct FrameData {
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;

    VkFence fence;
    VkSemaphore semaphore;
};

struct ImageData {
    VkImage image;
    VkImageView image_view;
    VkFramebuffer framebuffer;
    VkSemaphore semaphore;
};


class RendererBase {
protected:
    RendererBase();
    RendererBase(const RendererBase&) = delete;
    RendererBase(RendererBase&&) noexcept = delete;
    ~RendererBase();

    RendererBase& operator=(const RendererBase&) = delete;
    RendererBase& operator=(RendererBase&&) noexcept = delete;

    VkResult wait_all_fences() const noexcept;

    struct {
        VkInstance instance;
        VkSurfaceKHR surface;
        
        VkDevice device;
        VkQueue queue;

        VkRenderPass render_pass;

        std::array<FrameData, NUM_FRAMES_IN_FLIGHT> frame_data;

        VkSwapchainKHR swapchain;
        std::vector<ImageData> image_data;
    } d;
};
