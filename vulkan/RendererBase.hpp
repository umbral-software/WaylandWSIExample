#pragma once

#include "Swapchain.hpp"

#include <array>

inline constexpr size_t NUM_FRAMES_IN_FLIGHT = 2;

struct FrameData {
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;

    VkBuffer ui_index_buffer, ui_vertex_buffer;
    VmaAllocation ui_index_allocation, ui_vertex_allocation;

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

    struct {
        VkInstance instance;
        VkSurfaceKHR surface;
        
        VkDevice device;
        VmaAllocator allocator;

        VkDescriptorSetLayout ui_descriptor_set_layout, world_descriptor_set_layout;
        VkPipelineLayout world_pipeline_layout, ui_pipeline_layout;
        VkRenderPass render_pass;
        VkPipeline world_pipeline, ui_pipeline;

        VkDescriptorPool descriptor_pool;
        VkDescriptorSet world_descriptor_set, ui_descriptor_set;

        VkSampler bilinear_sampler;

        VkCommandPool staging_command_pool;
        VkFence staging_fence;
        VkBuffer staging_buffer;
        VmaAllocation staging_allocation;

        VkBuffer world_index_buffer, world_vertex_buffer, world_uniform_buffer;
        VmaAllocation world_index_allocation, world_vertex_allocation, world_uniform_allocation;

        VkImage ui_font_image;
        VmaAllocation ui_font_allocation;
        VkImageView ui_font_image_view;

        std::array<FrameData, NUM_FRAMES_IN_FLIGHT> frame_data;
    } d;

    Swapchain _swapchain;
};
