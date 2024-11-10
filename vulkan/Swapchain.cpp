#include "Swapchain.hpp"

#include "Common.hpp"

#include <volk.h>

#include <algorithm>
#include <array>
#include <memory>
#include <stdexcept>

static constexpr uint32_t DEFAULT_IMAGE_COUNT = 3;
static constexpr std::array DESIRED_COLOR_FORMATS {
    VK_FORMAT_R8G8B8A8_SRGB,
    VK_FORMAT_B8G8R8A8_SRGB
};
static constexpr std::array DESIRED_DEPTH_FORMATS {
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_X8_D24_UNORM_PACK32
};

static VkFormat select_depth_format(VkPhysicalDevice physical_device) {
    for (const auto format : DESIRED_DEPTH_FORMATS) {
        VkFormatProperties format_props;
        vkGetPhysicalDeviceFormatProperties(physical_device, format, &format_props);
        if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return format;
        }
    }

    throw std::runtime_error("No supported depth format");
}

static VkSurfaceFormatKHR select_surface_format(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    uint32_t num_formats;
    check_success(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &num_formats, nullptr));
    const auto surface_formats = std::make_unique_for_overwrite<VkSurfaceFormatKHR[]>(num_formats);
    check_success(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &num_formats, surface_formats.get()));

    for (uint32_t i = 0; i < num_formats; ++i) {
        const auto& surface_format = surface_formats[i];
        if (surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            for (const auto format : DESIRED_COLOR_FORMATS) {
                if (format == surface_format.format) {
                    return surface_format;
                }
            }
        }
    }

    throw std::runtime_error("No supported surface format");
}

bool Swapchain::acquire(VkSemaphore semaphore) {
    const auto result = vkAcquireNextImageKHR(_device, d.swapchain, UINT64_MAX, semaphore, nullptr, &_image_index);
    switch (result) {
    case VK_SUCCESS:
        return true;
    case VK_SUBOPTIMAL_KHR:
        _rebuild_required = true;
        return true;
    case VK_ERROR_OUT_OF_DATE_KHR:
        _rebuild_required = true;
        return false;
    default:
        throw BadVkResult(result);
    }
}

void Swapchain::destroy(bool destroy_current_swapchain) {
    vkDestroyImageView(_device, d.depth_view, nullptr);
    vmaDestroyImage(_allocator, d.depth_image, d.depth_allocation);

    for (auto& image_data : d.image_data) {
        vkDestroySemaphore(_device, image_data.semaphore, nullptr);
        vkDestroyFramebuffer(_device, image_data.framebuffer, nullptr);
        vkDestroyImageView(_device, image_data.image_view, nullptr);
    }
    d.image_data.clear();

    vkDestroySwapchainKHR(_device, d.old_swapchain, nullptr);
    if (destroy_current_swapchain) {
        vkDestroySwapchainKHR(_device, d.swapchain, nullptr);
    }
    d.old_swapchain = d.swapchain;
}

VkFormat Swapchain::depth_format() const noexcept {
    return _depth_format;
}

VkFormat Swapchain::format() const noexcept {
    return _format.format;
}

ImageData& Swapchain::image_data() {
    return d.image_data[_image_index];
}

const ImageData& Swapchain::image_data() const {
    return d.image_data[_image_index];
}

void Swapchain::init(VkDevice device, VmaAllocator allocator, VkSurfaceKHR surface, VkPhysicalDevice physical_device) {
    _device = device;
    _allocator = allocator;
    _surface = surface;
    _physical_device = physical_device;

    _format = select_surface_format(_physical_device, _surface);
    _depth_format = select_depth_format(_physical_device);

    _rebuild_required = true;
}

void Swapchain::present(VkQueue queue) {
    const VkPresentInfoKHR present_info {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &image_data().semaphore,
        .swapchainCount = 1,
        .pSwapchains = &d.swapchain,
        .pImageIndices = &_image_index
    };
    const auto result = vkQueuePresentKHR(queue, &present_info);
    switch (result) {
    case VK_SUCCESS:
        break;
    case VK_SUBOPTIMAL_KHR:
    case VK_ERROR_OUT_OF_DATE_KHR:
        _rebuild_required = true;
        break;
    default:
        throw BadVkResult(result);
    }
}

void Swapchain::rebuild(const std::pair<uint32_t, uint32_t>& window_size, VkRenderPass render_pass) {
    printf("%ux%u\n", window_size.first, window_size.second);

    const VkSurfacePresentModeEXT present_mode {
        .sType = VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_EXT,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR // Always supported
    };
    const VkPhysicalDeviceSurfaceInfo2KHR surface_info {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
        .pNext = &present_mode,
        .surface = _surface
    };
    VkSurfaceCapabilities2KHR surface_caps2 {
        .sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR,
    };
    check_success(vkGetPhysicalDeviceSurfaceCapabilities2KHR(_physical_device, &surface_info, &surface_caps2));

    auto image_count = std::max(surface_caps2.surfaceCapabilities.minImageCount + 1, DEFAULT_IMAGE_COUNT);
    if (surface_caps2.surfaceCapabilities.maxImageCount) {
        image_count = std::max(image_count, surface_caps2.surfaceCapabilities.maxImageCount);
    }
    
    VkCompositeAlphaFlagBitsKHR compositeAlpha;
    if (surface_caps2.surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
        compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    } else if (surface_caps2.surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) {
        compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    } else {
        throw std::runtime_error("No supported composite alpha");
    }

    _size = {
        std::clamp(window_size.first, surface_caps2.surfaceCapabilities.minImageExtent.width, surface_caps2.surfaceCapabilities.maxImageExtent.width),
        std::clamp(window_size.second, surface_caps2.surfaceCapabilities.minImageExtent.height, surface_caps2.surfaceCapabilities.maxImageExtent.height),
    };

    destroy();
    const VkSwapchainCreateInfoKHR swapchain_create_info {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = _surface,
        .minImageCount = image_count,
        .imageFormat = _format.format,
        .imageColorSpace = _format.colorSpace,
        .imageExtent = _size,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = surface_caps2.surfaceCapabilities.currentTransform,
        .compositeAlpha = compositeAlpha,
        .presentMode = present_mode.presentMode,
        .clipped = true,
        .oldSwapchain = d.old_swapchain
    };
    check_success(vkCreateSwapchainKHR(_device, &swapchain_create_info, nullptr, &d.swapchain));

    const VkImageCreateInfo depth_image_create_info {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = _depth_format,
        .extent = { _size.width, _size.height, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    };
    const VmaAllocationCreateInfo depth_image_allocate_info {
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        .priority = RENDER_TARGET_PRIORITY
    };
    vmaCreateImage(_allocator, &depth_image_create_info, &depth_image_allocate_info, &d.depth_image, &d.depth_allocation, nullptr);

    const VkImageViewCreateInfo depth_view_create_info {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = d.depth_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = _depth_format,
        .subresourceRange = { 
            VK_IMAGE_ASPECT_DEPTH_BIT, 
            0, 1, 
            0, 1
        }
    };
    check_success(vkCreateImageView(_device, &depth_view_create_info, nullptr, &d.depth_view));

    uint32_t num_images;
    check_success(vkGetSwapchainImagesKHR(_device, d.swapchain, &num_images, nullptr));
    const auto images = std::make_unique_for_overwrite<VkImage[]>(num_images);
    check_success(vkGetSwapchainImagesKHR(_device, d.swapchain, &num_images, images.get()));

    d.image_data.resize(num_images);
    for (uint32_t i = 0; i < num_images; ++i) {
        auto& image_data = d.image_data[i];
        image_data.image = images[i];

        const VkImageViewCreateInfo image_view_create_info {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = image_data.image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = _format.format,
            .subresourceRange = { 
                VK_IMAGE_ASPECT_COLOR_BIT, 
                0, 1, 
                0, 1
            }
        };
        check_success(vkCreateImageView(_device, &image_view_create_info, nullptr, &image_data.image_view));

        const std::array attachments { image_data.image_view, d.depth_view };
        const VkFramebufferCreateInfo framebuffer_create_info {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = render_pass,
            .attachmentCount = attachments.size(),
            .pAttachments = attachments.data(),
            .width = _size.width,
            .height = _size.height,
            .layers = 1
        };
        check_success(vkCreateFramebuffer(_device, &framebuffer_create_info, nullptr, &image_data.framebuffer));

        const VkSemaphoreCreateInfo semaphore_create_info {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };
        check_success(vkCreateSemaphore(_device, &semaphore_create_info, nullptr, &image_data.semaphore));

        _rebuild_required = false;
    }
}

bool Swapchain::rebuild_required() const noexcept {
    return _rebuild_required;
}

VkExtent2D Swapchain::size() const noexcept {
    return _size;
}
