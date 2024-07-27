#pragma once

#include <vk_mem_alloc.h>

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
        VmaAllocator allocator;
        VkQueue queue;

        VkDescriptorSetLayout descriptor_set_layout;
        VkPipelineLayout pipeline_layout;
        VkRenderPass render_pass;

        VkDescriptorPool descriptor_pool;
        VkDescriptorSet descriptor_set;
        VkPipeline pipeline;

        VkBuffer index_buffer, vertex_buffer, uniform_buffer;
        VmaAllocation index_allocation, vertex_allocation, uniform_allocation;

        std::array<FrameData, NUM_FRAMES_IN_FLIGHT> frame_data;

        VkSwapchainKHR swapchain;
        VkImage depth_image;
        VmaAllocation depth_allocation;
        VkImageView depth_view;

        std::vector<ImageData> image_data;
    } d;
};
