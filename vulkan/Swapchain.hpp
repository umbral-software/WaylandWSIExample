#pragma once

#include "SwapchainBase.hpp"

class Swapchain : private SwapchainBase {
public:
    bool acquire(VkSemaphore semaphore);

    void destroy(bool destroy_current_swapchain = false);

    VkFormat depth_format() const noexcept;
    VkFormat format() const noexcept;

    VkSwapchainKHR handle() noexcept;

    ImageData& image_data();
    const ImageData& image_data() const;

    void init(VkDevice device, VmaAllocator allocator, VkSurfaceKHR surface, VkPhysicalDevice physical_device);

    void present(VkQueue queue);

    bool rebuild_required() const noexcept;
    void rebuild(const VkExtent2D& size, VkRenderPass render_pass);

    VkExtent2D size() const noexcept;

private:
    VkSurfaceKHR _surface;
    VkPhysicalDevice _physical_device;

    VkSurfaceFormatKHR _format;
    VkFormat _depth_format;

    VkExtent2D _size;
    uint32_t _image_index;
    bool _rebuild_required;
};
