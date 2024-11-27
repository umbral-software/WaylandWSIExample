#include "RendererBase.hpp"

#include <volk.h>

RendererBase::RendererBase()
    :d{}
{}

RendererBase::~RendererBase() {
    if (d.device) {
        for (const auto& frame_data : d.frame_data) {
            vkDestroySemaphore(d.device, frame_data.semaphore, nullptr);   
            vkDestroyFence(d.device, frame_data.fence, nullptr);

            vmaDestroyBuffer(d.allocator, frame_data.ui_vertex_buffer, frame_data.ui_vertex_allocation);
            vmaDestroyBuffer(d.allocator, frame_data.ui_index_buffer, frame_data.ui_index_allocation);

            vkDestroyCommandPool(d.device, frame_data.command_pool, nullptr);        
        }

        vkDestroyImageView(d.device, d.ui_font_image_view, nullptr);
        vmaDestroyImage(d.allocator, d.ui_font_image, d.ui_font_allocation);

        vmaDestroyBuffer(d.allocator, d.world_uniform_buffer, d.world_uniform_allocation);
        vmaDestroyBuffer(d.allocator, d.world_vertex_buffer, d.world_vertex_allocation);
        vmaDestroyBuffer(d.allocator, d.world_index_buffer, d.world_index_allocation);

        vmaDestroyBuffer(d.allocator, d.staging_buffer, d.staging_allocation);
        vkDestroyFence(d.device, d.staging_fence, nullptr);
        vkDestroyCommandPool(d.device, d.staging_command_pool, nullptr);

        vkDestroyPipeline(d.device, d.ui_pipeline, nullptr);
        vkDestroyPipeline(d.device, d.world_pipeline, nullptr);

        vkDestroySampler(d.device, d.bilinear_sampler, nullptr);

        vkDestroyDescriptorPool(d.device, d.descriptor_pool, nullptr);

        vkDestroyRenderPass(d.device, d.render_pass, nullptr);
        vkDestroyPipelineLayout(d.device, d.ui_pipeline_layout, nullptr);
        vkDestroyPipelineLayout(d.device, d.world_pipeline_layout, nullptr);
        vkDestroyDescriptorSetLayout(d.device, d.ui_descriptor_set_layout, nullptr);
        vkDestroyDescriptorSetLayout(d.device, d.world_descriptor_set_layout, nullptr);

        vmaDestroyAllocator(d.allocator);
        vkDestroyDevice(d.device, nullptr);
    }

    if (d.instance) {
        vkDestroySurfaceKHR(d.instance, d.surface, nullptr);
        vkDestroyInstance(d.instance, nullptr);
        volkFinalize();
    }
}

