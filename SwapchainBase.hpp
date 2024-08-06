#pragma once

#include <vk_mem_alloc.h>
#include <vector>

struct ImageData {
    VkImage image;
    VkImageView image_view;
    VkFramebuffer framebuffer;
    VkSemaphore semaphore;
};

class SwapchainBase {
protected:
    SwapchainBase();
    SwapchainBase(const SwapchainBase&) = delete;
    SwapchainBase(SwapchainBase&&) noexcept = delete;
    ~SwapchainBase() = default;

    SwapchainBase& operator=(const SwapchainBase&) = delete;
    SwapchainBase& operator=(SwapchainBase&&) noexcept = delete;

    void destroy();

protected:
    VkDevice _device;
    VmaAllocator _allocator;

    struct {
        VkSwapchainKHR swapchain, old_swapchain;
        VkImage depth_image;
        VmaAllocation depth_allocation;
        VkImageView depth_view;

        std::vector<ImageData> image_data;
    } d;
};
