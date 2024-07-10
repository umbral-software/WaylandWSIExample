#include "Display.hpp"
#include "Window.hpp"

#include <volk.h>
#include <vulkan/vk_enum_string_helper.h>

#include <array>
#include <vector>

static constexpr size_t NUM_FRAMES_IN_FLIGHT = 2;

constexpr void check_success(VkResult result) {
    if (result) {
        throw std::runtime_error(string_VkResult(result));
    }
}

struct FrameData {
    VkFence fence;
    VkSemaphore semaphore;
};

struct ImageData {
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
        
        VkPhysicalDevice physical_device;
        uint32_t queue_family_index;

        VkDevice device;
        VkQueue queue;

        std::array<FrameData, NUM_FRAMES_IN_FLIGHT> frame_data;

        VkSwapchainKHR swapchain;
        std::vector<ImageData> image_data;
    } d;
};

RendererBase::RendererBase()
    :d{}
{}

RendererBase::~RendererBase() {
    if (d.device) {
        std::array<VkFence, NUM_FRAMES_IN_FLIGHT> all_fences;
        for (size_t i = 0; i < NUM_FRAMES_IN_FLIGHT; ++i) {
            all_fences[i] = d.frame_data[i].fence;
        }
        vkWaitForFences(d.device, all_fences.size(), all_fences.data(), true, UINT64_MAX);

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

class Renderer : private RendererBase {
public:
    Renderer(Window& window);

    FrameData& frame() noexcept;
    ImageData& image() noexcept;
    void render();

private:
    const Window& _window;
    size_t _frame_index;
    uint32_t _image_index;
};

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

    uint32_t num_physical_devices;
    check_success(vkEnumeratePhysicalDevices(d.instance, &num_physical_devices, nullptr));
    auto physical_devices = std::make_unique_for_overwrite<VkPhysicalDevice[]>(num_physical_devices);
    check_success(vkEnumeratePhysicalDevices(d.instance, &num_physical_devices, physical_devices.get()));
    
    std::tie(d.physical_device, d.queue_family_index) = std::make_pair(physical_devices[0], 0); // FIXME

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

    const VkSemaphoreCreateInfo semaphore_create_info {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    for (auto& frame_data : d.frame_data) {
        const VkFenceCreateInfo signalled_fence_create_info {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };
        check_success(vkCreateFence(d.device, &signalled_fence_create_info, nullptr, &frame_data.fence));
        check_success(vkCreateSemaphore(d.device, &semaphore_create_info, nullptr, &frame_data.semaphore));
    }
    _frame_index = d.frame_data.size();

    VkSurfaceCapabilitiesKHR surface_caps;
    check_success(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(d.physical_device, d.surface, &surface_caps));

    uint32_t num_formats;
    check_success(vkGetPhysicalDeviceSurfaceFormatsKHR(d.physical_device, d.surface, &num_formats, nullptr));
    const auto surface_formats = std::make_unique_for_overwrite<VkSurfaceFormatKHR[]>(num_formats);
    check_success(vkGetPhysicalDeviceSurfaceFormatsKHR(d.physical_device, d.surface, &num_formats, surface_formats.get()));

    const VkSwapchainCreateInfoKHR swapchain_create_info {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = d.surface,
        .minImageCount = surface_caps.minImageCount, // FIXME
        .imageFormat = surface_formats[0].format, // FIXME
        .imageColorSpace = surface_formats[0].colorSpace, // FIXME
        .imageExtent = { 800, 600 }, // FIXME
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = surface_caps.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, // FIXME
        .presentMode = VK_PRESENT_MODE_FIFO_KHR, // FIFO is always suppported
        .clipped = true
    };
    check_success(vkCreateSwapchainKHR(d.device, &swapchain_create_info, nullptr, &d.swapchain));

    uint32_t num_images;
    check_success(vkGetSwapchainImagesKHR(d.device, d.swapchain, &num_images, nullptr));
    const auto images = std::make_unique_for_overwrite<VkImage[]>(num_images);
    check_success(vkGetSwapchainImagesKHR(d.device, d.swapchain, &num_images, images.get()));

    d.image_data.resize(num_images);
    for (auto& image_data : d.image_data) {
        check_success(vkCreateSemaphore(d.device, &semaphore_create_info, nullptr, &image_data.semaphore));
    }
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
    check_success(vkAcquireNextImageKHR(d.device, d.swapchain, UINT64_MAX, frame().semaphore, nullptr, &_image_index));

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
    check_success(vkQueuePresentKHR(d.queue, &present_info)) ;
}

int main() {
    Display display;
    Window window(display);
    Renderer renderer(window);

    while (!window.should_close()) {
        display.poll_events();
        renderer.render();
    }
}
