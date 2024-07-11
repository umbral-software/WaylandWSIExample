#include "RendererBase.hpp"

#include <volk.h>

RendererBase::RendererBase()
    :d{}
{}

RendererBase::~RendererBase() {
    if (d.device) {
        wait_all_fences();

        for (const auto& image_data : d.image_data) {
            vkDestroySemaphore(d.device, image_data.semaphore, nullptr);   
        }
        vkDestroySwapchainKHR(d.device, d.swapchain, nullptr);

        for (const auto& frame_data : d.frame_data) {
            vkDestroySemaphore(d.device, frame_data.semaphore, nullptr);   
            vkDestroyFence(d.device, frame_data.fence, nullptr);        
        }
        vkDestroyDevice(d.device, nullptr);
    }

    if (d.instance) {
        vkDestroySurfaceKHR(d.instance, d.surface, nullptr);
        vkDestroyInstance(d.instance, nullptr);
    }
}

VkResult RendererBase::wait_all_fences() const noexcept {
    std::array<VkFence, NUM_FRAMES_IN_FLIGHT> all_fences;
    for (size_t i = 0; i < NUM_FRAMES_IN_FLIGHT; ++i) {
        all_fences[i] = d.frame_data[i].fence;
    }
    return vkWaitForFences(d.device, all_fences.size(), all_fences.data(), true, UINT64_MAX);
}
