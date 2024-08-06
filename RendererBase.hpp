#pragma once

#include "Swapchain.hpp"

#include <array>

#define NUM_FRAMES_IN_FLIGHT 2

struct FrameData {
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;

    VkFence fence;
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
        VmaAllocator allocator;

        VkDescriptorSetLayout descriptor_set_layout;
        VkPipelineLayout pipeline_layout;
        VkRenderPass render_pass;

        VkDescriptorPool descriptor_pool;
        VkDescriptorSet descriptor_set;
        VkPipeline pipeline;

        VkBuffer index_buffer, vertex_buffer, uniform_buffer;
        VmaAllocation index_allocation, vertex_allocation, uniform_allocation;

        std::array<FrameData, NUM_FRAMES_IN_FLIGHT> frame_data;
    } d;

    Swapchain _swapchain;
};
