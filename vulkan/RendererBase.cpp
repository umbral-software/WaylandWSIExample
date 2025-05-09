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
            vkDestroyCommandPool(d.device, frame_data.command_pool, nullptr);        
        }

        vmaDestroyBuffer(d.allocator, d.uniform_buffer, d.uniform_allocation);
        vmaDestroyBuffer(d.allocator, d.vertex_buffer, d.vertex_allocation);
        vmaDestroyBuffer(d.allocator, d.index_buffer, d.index_allocation);

        vkDestroyPipeline(d.device, d.pipeline, nullptr);
        vkDestroyDescriptorPool(d.device, d.descriptor_pool, nullptr);

        vkDestroyRenderPass(d.device, d.render_pass, nullptr);
        vkDestroyPipelineLayout(d.device, d.pipeline_layout, nullptr);
        vkDestroyDescriptorSetLayout(d.device, d.descriptor_set_layout, nullptr);

        vmaDestroyAllocator(d.allocator);
        vkDestroyDevice(d.device, nullptr);
    }

    if (d.instance) {
        vkDestroySurfaceKHR(d.instance, d.surface, nullptr);
        vkDestroyInstance(d.instance, nullptr);
        volkFinalize();
    }
}
