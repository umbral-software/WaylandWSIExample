#include "Renderer.hpp"

#include "Window.hpp"

#include <volk.h>
#include <vulkan/vk_enum_string_helper.h>

#include <algorithm>

static constexpr uint32_t DEFAULT_IMAGE_COUNT = 3;

static constexpr void check_success(VkResult result) {
    if (result) {
        throw std::runtime_error(string_VkResult(result));
    }
}

static std::pair<VkPhysicalDevice, uint32_t> select_device_and_queue(VkInstance instance, VkSurfaceKHR surface) {
    uint32_t num_physical_devices;
    check_success(vkEnumeratePhysicalDevices(instance, &num_physical_devices, nullptr));
    auto physical_devices = std::make_unique_for_overwrite<VkPhysicalDevice[]>(num_physical_devices);
    check_success(vkEnumeratePhysicalDevices(instance, &num_physical_devices, physical_devices.get()));

    for (uint32_t i = 0; i < num_physical_devices; ++i) {
        const auto physical_device = physical_devices[i];

        uint32_t num_queue_families;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &num_queue_families, nullptr);
        const auto queue_family_props = std::make_unique_for_overwrite<VkQueueFamilyProperties[]>(num_queue_families);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &num_queue_families, queue_family_props.get());

        for (uint32_t j = 0; j < num_queue_families; ++j) {
            VkBool32 supported;
            check_success(vkGetPhysicalDeviceSurfaceSupportKHR(physical_devices[i], j, surface, &supported));

            if (queue_family_props[j].queueFlags & VK_QUEUE_GRAPHICS_BIT && supported) {
                return { physical_devices[i], j };
            }
        }
    }

    throw std::runtime_error("No supported device and queue");
}

static VkSurfaceFormatKHR select_surface_format(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    uint32_t num_formats;
    check_success(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &num_formats, nullptr));
    const auto surface_formats = std::make_unique_for_overwrite<VkSurfaceFormatKHR[]>(num_formats);
    check_success(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &num_formats, surface_formats.get()));

    for (uint32_t i = 0; i < num_formats; ++i) {
        const auto& surface_format = surface_formats[i];
        if (surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            switch (surface_format.format) {
            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_B8G8R8A8_SRGB:
                return surface_format;
            default:
                break;
            }
        }
    }

    throw std::runtime_error("No supported surface format");
}

Renderer::Renderer(Window& window)
    :_window(window)
{
    check_success(volkInitialize());

    const VkApplicationInfo application_info {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .apiVersion = VK_API_VERSION_1_3
    };
    const std::array instance_extensions { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME };
    const VkInstanceCreateInfo instance_create_info {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &application_info,
        .enabledExtensionCount = instance_extensions.size(),
        .ppEnabledExtensionNames = instance_extensions.data()
    };
    check_success(vkCreateInstance(&instance_create_info, nullptr, &d.instance));
    volkLoadInstanceOnly(d.instance);

    const VkWaylandSurfaceCreateInfoKHR surface_create_info {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .display = window.display(),
        .surface = window.surface()
    };
    check_success(vkCreateWaylandSurfaceKHR(d.instance, &surface_create_info, nullptr, &d.surface));

    std::tie(d.physical_device, d.queue_family_index) = select_device_and_queue(d.instance, d.surface);

    d.surface_format = select_surface_format(d.physical_device, d.surface);

    const std::array device_extensions { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    const float queue_priority = 1.0f;
    const VkDeviceQueueCreateInfo queue_create_info {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = d.queue_family_index,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority
    };
    const VkDeviceCreateInfo device_create_info {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue_create_info,
        .enabledExtensionCount = device_extensions.size(),
        .ppEnabledExtensionNames = device_extensions.data()
    };
    check_success(vkCreateDevice(d.physical_device, &device_create_info, nullptr, &d.device));
    volkLoadDevice(d.device);

    vkGetDeviceQueue(d.device, d.queue_family_index, 0, &d.queue);

    for (auto& frame_data : d.frame_data) {
        const VkFenceCreateInfo signalled_fence_create_info {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };
        check_success(vkCreateFence(d.device, &signalled_fence_create_info, nullptr, &frame_data.fence));

        const VkSemaphoreCreateInfo semaphore_create_info {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };
        check_success(vkCreateSemaphore(d.device, &semaphore_create_info, nullptr, &frame_data.semaphore));
    }
    _frame_index = d.frame_data.size();

    rebuild_swapchain();
}

FrameData& Renderer::frame() noexcept {
    return d.frame_data[_frame_index];
}

ImageData& Renderer::image() noexcept {
    return d.image_data[_image_index];
}

void Renderer::render() {
    _frame_index = (_frame_index + 1) % d.frame_data.size();
    check_success(vkWaitForFences(d.device, 1, &frame().fence, true, UINT64_MAX));
    const auto acquire_result = vkAcquireNextImageKHR(d.device, d.swapchain, UINT64_MAX, frame().semaphore, nullptr, &_image_index);

    bool swapchain_usable, rebuild_required;
    switch (acquire_result) {
    case VK_SUCCESS:
        swapchain_usable = true;
        rebuild_required = false;
        break;
    case VK_SUBOPTIMAL_KHR:
        swapchain_usable = true;
        rebuild_required = true;
        break;
    case VK_ERROR_OUT_OF_DATE_KHR:
        swapchain_usable = false;
        rebuild_required = true;
        break;
    default:
        throw std::runtime_error(string_VkResult(acquire_result));
    }

    if (swapchain_usable) {
        const VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        const VkSubmitInfo submit_info {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &frame().semaphore,
            .pWaitDstStageMask = &wait_stage_mask,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &image().semaphore
        };
        check_success(vkResetFences(d.device, 1, &frame().fence));
        check_success(vkQueueSubmit(d.queue, 1, &submit_info, frame().fence));

        const VkPresentInfoKHR present_info {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &image().semaphore,
            .swapchainCount = 1,
            .pSwapchains = &d.swapchain,
            .pImageIndices = &_image_index
        };
        const auto present_result = vkQueuePresentKHR(d.queue, &present_info);
        switch (present_result) {
        case VK_SUCCESS:
            break;
        case VK_SUBOPTIMAL_KHR:
        case VK_ERROR_OUT_OF_DATE_KHR:
            rebuild_required = true;
            break;
        default:
            throw std::runtime_error(string_VkResult(present_result));
        }
    }

    if (rebuild_required || _swapchain_size != _window.size()) {
        rebuild_swapchain();
    }
}

void Renderer::rebuild_swapchain() {
    VkSurfaceCapabilitiesKHR surface_caps;
    check_success(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(d.physical_device, d.surface, &surface_caps));

    auto image_count = std::max(surface_caps.minImageCount + 1, DEFAULT_IMAGE_COUNT);
    if (surface_caps.maxImageCount) {
        image_count = std::max(image_count, surface_caps.maxImageCount);
    }
    
    VkCompositeAlphaFlagBitsKHR compositeAlpha;
    if (surface_caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
        compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    } else if (surface_caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) {
        compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    } else {
        throw std::runtime_error("No supported composite alpha");
    }

    const auto window_size = _window.size();

    _swapchain_size = {
        std::clamp(window_size.first, surface_caps.minImageExtent.width, surface_caps.maxImageExtent.width),
        std::clamp(window_size.second, surface_caps.minImageExtent.height, surface_caps.maxImageExtent.height),
    };

    const VkSwapchainCreateInfoKHR swapchain_create_info {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = d.surface,
        .minImageCount = image_count,
        .imageFormat = d.surface_format.format,
        .imageColorSpace = d.surface_format.colorSpace,
        .imageExtent = { _swapchain_size.first, _swapchain_size.second },
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = surface_caps.currentTransform,
        .compositeAlpha = compositeAlpha,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR, // FIFO is always suppported
        .clipped = true,
        .oldSwapchain = d.swapchain
    };
    check_success(wait_all_fences());
    check_success(vkCreateSwapchainKHR(d.device, &swapchain_create_info, nullptr, &d.swapchain));
    vkDestroySwapchainKHR(d.device, swapchain_create_info.oldSwapchain, nullptr);

    uint32_t num_images;
    check_success(vkGetSwapchainImagesKHR(d.device, d.swapchain, &num_images, nullptr));
    const auto images = std::make_unique_for_overwrite<VkImage[]>(num_images);
    check_success(vkGetSwapchainImagesKHR(d.device, d.swapchain, &num_images, images.get()));

    for (auto& image_data : d.image_data) {
        vkDestroySemaphore(d.device, image_data.semaphore, nullptr);
    }
    d.image_data.clear();

    d.image_data.resize(num_images);
    for (auto& image_data : d.image_data) {
        const VkSemaphoreCreateInfo semaphore_create_info {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };
        check_success(vkCreateSemaphore(d.device, &semaphore_create_info, nullptr, &image_data.semaphore));
    }
}
