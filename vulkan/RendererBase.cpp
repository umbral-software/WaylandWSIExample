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

VkResult RendererBase::wait_all_fences() const noexcept {
    std::array<VkFence, NUM_FRAMES_IN_FLIGHT> all_fences;
    for (size_t i = 0; i < NUM_FRAMES_IN_FLIGHT; ++i) {
        all_fences[i] = d.frame_data[i].fence;
    }
    return vkWaitForFences(d.device, all_fences.size(), all_fences.data(), true, UINT64_MAX);
}
