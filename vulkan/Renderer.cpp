#include "Renderer.hpp"

#include "Common.hpp"
#include "wayland/Window.hpp"

#include <glm/gtc/reciprocal.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <volk.h>

#include <cstring>
#include <filesystem>
#include <fstream>

using namespace std::literals;

struct PhysicalDeviceInformation {
    VkPhysicalDevice physical_device;
    uint32_t graphics_queue, compute_queue, transfer_queue;

    bool graphics_queue_supports_presentation;
    bool has_memory_priority;
    bool has_pageable_device_local_memory;
    bool has_maintenance_5;
    bool has_synchronization_2;
};

struct MatrixUniforms {
    glm::mat4 modelview;
    glm::mat4 projection;
};

struct Vertex {
    std::array<float, 3> position;
    std::array<uint8_t, 4> color;
};

static const std::array REQUIRED_INSTANCE_EXTENSIONS {
    VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME,
    VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
};

static constexpr float FIELD_OF_VIEW = glm::radians(90.0f);
static constexpr float NEAR_CLIP_PLANE = 0.01f;

static constexpr std::array<uint16_t, 3> INDICES {{
    0, 1, 2
}};
static constexpr std::array<Vertex, 3> VERTICES {{
    {{0.0f,  0.0f, 0.0f}, { 255,   0,   0, 255 }},
    {{2.0f,  0.0f, 0.0f}, {   0, 255,   0, 255 }},
    {{1.0f,  1.5f, 0.0f}, {   0,   0, 255, 255 }},
}};

static std::partial_ordering operator<=>(const VkExtent2D& extent, const std::pair<uint32_t, uint32_t>& pair) noexcept {
    const std::weak_ordering width_comparison = extent.width <=> pair.first;
    const std::weak_ordering height_comparison = extent.height <=> pair.second;

    if (width_comparison == height_comparison) {
        return width_comparison;
    }
    if (width_comparison == std::weak_ordering::equivalent) {
        return height_comparison;
    }
    if (height_comparison == std::weak_ordering::equivalent) {
        return width_comparison;
    }
    return std::partial_ordering::unordered;
}

static bool operator==(const VkExtent2D& extent, const std::pair<uint32_t, uint32_t>& pair) noexcept {
    return std::partial_ordering::equivalent == (extent <=> pair);
}

static glm::mat4 infinitePerspectiveFovReverse(float fovx, float aspect, float zNear) {
    const float w = glm::cot(0.5f * fovx);
    const float h = w * aspect;
    glm::mat4 result = glm::zero<glm::mat4>();
    result[0][0] = w;
    result[1][1] = h;
    result[2][2] = 0.0f;
    result[2][3] = 1.0f;
    result[3][2] = zNear;
    return result;
}

static uint32_t find_queue(VkPhysicalDevice physical_device, VkQueueFlags required_flags, VkQueueFlags prohibited_flags) {
    uint32_t num_queue_families;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &num_queue_families, nullptr);
    auto queue_family_props = std::make_unique_for_overwrite<VkQueueFamilyProperties[]>(num_queue_families);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &num_queue_families, queue_family_props.get());

    for (uint32_t i = 0; i < num_queue_families; ++i) {
        if (required_flags == (queue_family_props[i].queueFlags & required_flags)
         && 0 == (queue_family_props[i].queueFlags & prohibited_flags))
        {
            return i;
        }
    }

    return -1;
}

static std::vector<uint8_t> load_file(std::filesystem::path path) {
    std::ifstream file(path, std::ios::binary);
    std::vector<uint8_t> ret;   
    std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), std::back_inserter(ret));
    return ret;
}

static std::vector<uint32_t> load_shader(std::filesystem::path path) {
    path += ".spv";
    const auto raw = load_file("shaders" / path);
    std::vector<uint32_t> ret(raw.size() / sizeof(uint32_t));
    memcpy(ret.data(), raw.data(), ret.size() * sizeof(uint32_t));
    return ret;
}

static std::vector<PhysicalDeviceInformation> get_physical_device_info(VkInstance instance, VkSurfaceKHR surface) {
    uint32_t num_physical_devices;
    check_success(vkEnumeratePhysicalDevices(instance, &num_physical_devices, nullptr));
    auto physical_devices = std::make_unique_for_overwrite<VkPhysicalDevice[]>(num_physical_devices);
    check_success(vkEnumeratePhysicalDevices(instance, &num_physical_devices, physical_devices.get()));

    std::vector<PhysicalDeviceInformation> ret(num_physical_devices);
    for (uint32_t i = 0; i < num_physical_devices; ++i) {
        auto& device_info = ret[i];
        const auto physical_device = physical_devices[i];

        device_info.physical_device = physical_device;
        device_info.graphics_queue = find_queue(physical_device, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0);
        device_info.compute_queue = find_queue(physical_device, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
        device_info.transfer_queue = find_queue(physical_device, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

        uint32_t num_device_extensions;
        check_success(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &num_device_extensions, nullptr));
        const auto device_extension_properties = std::make_unique_for_overwrite<VkExtensionProperties[]>(num_device_extensions);
        check_success(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &num_device_extensions, device_extension_properties.get()));

        bool has_ext_memory_priority = false;
        bool has_ext_pageable_device_local_memory = false;
        bool has_khr_maintenance_5 = false;
        bool has_khr_swapchain = false;
        for (uint32_t j = 0; j < num_device_extensions; ++j) {
            const auto extension_name = device_extension_properties[j].extensionName;
            if (!strcmp(extension_name, VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME)) {
                has_ext_memory_priority = true;
            } else if (!strcmp(extension_name, VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME)) {
                has_ext_pageable_device_local_memory = true;
            } else if (!strcmp(extension_name, VK_KHR_MAINTENANCE_5_EXTENSION_NAME)) {
                has_khr_maintenance_5 = true;
            } else if (!strcmp(extension_name, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
                has_khr_swapchain = true;
            }
        }

        if (has_khr_swapchain && device_info.graphics_queue != -1) {
            VkBool32 supported;
            check_success(vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, device_info.graphics_queue, surface, &supported));
            device_info.graphics_queue_supports_presentation = !!supported;
        }

        void *optional_pnext_chain = nullptr;

        VkPhysicalDeviceMemoryPriorityFeaturesEXT memory_priority_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT,
            .pNext = &memory_priority_features
        };
        if (has_ext_memory_priority) {
            std::swap(optional_pnext_chain, memory_priority_features.pNext);
        }

        VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT pagable_device_local_memory_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PAGEABLE_DEVICE_LOCAL_MEMORY_FEATURES_EXT,
            .pNext = &pagable_device_local_memory_features
        };
        if (has_ext_pageable_device_local_memory) {
            std::swap(optional_pnext_chain, pagable_device_local_memory_features.pNext);
        }

        VkPhysicalDeviceMaintenance5FeaturesKHR maintenance_5_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR,
            .pNext = &maintenance_5_features
        };
        if (has_khr_maintenance_5) {
            std::swap(optional_pnext_chain, maintenance_5_features.pNext);
        }

        VkPhysicalDeviceVulkan13Features vulkan_1_3_features {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = optional_pnext_chain  
        };
        VkPhysicalDeviceFeatures2 physical_device_features {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &vulkan_1_3_features
        };
        vkGetPhysicalDeviceFeatures2(physical_device, &physical_device_features);

        device_info.has_memory_priority = memory_priority_features.memoryPriority;
        device_info.has_pageable_device_local_memory = pagable_device_local_memory_features.pageableDeviceLocalMemory;
        device_info.has_maintenance_5 = maintenance_5_features.maintenance5;
        device_info.has_synchronization_2 = vulkan_1_3_features.synchronization2;
    }

    return ret;
}

static PhysicalDeviceInformation select_physical_device(
    VkInstance instance, VkSurfaceKHR surface
) {
    for (const auto& device_info : get_physical_device_info(instance, surface)) {
        bool is_valid = true;

        if (!device_info.has_maintenance_5) {
            is_valid = false;
        }
        if (!device_info.has_synchronization_2) {
            is_valid = false;
        }
        if (!device_info.graphics_queue_supports_presentation) {
            is_valid = false;
        }

        if (is_valid) {
            return device_info;
        }
    }
    throw std::runtime_error("No supported device and queue");
}

Renderer::Renderer(Window& window)
    :_window(window)
{
    check_success(volkInitialize());

    uint32_t supported_api_version;
    check_success(vkEnumerateInstanceVersion(&supported_api_version));
    if (supported_api_version < VK_API_VERSION_1_3) {
        throw std::runtime_error("Instance does not support vulkan 1.3");
    }

    uint32_t num_instance_extensions;
    check_success(vkEnumerateInstanceExtensionProperties(nullptr, &num_instance_extensions, nullptr));
    const auto instance_extension_properties = std::make_unique_for_overwrite<VkExtensionProperties[]>(num_instance_extensions);
    check_success(vkEnumerateInstanceExtensionProperties(nullptr, &num_instance_extensions, instance_extension_properties.get()));

    for (const auto extension : REQUIRED_INSTANCE_EXTENSIONS) {
        bool found = false;
        for (uint32_t i = 0; i < num_instance_extensions; ++i) {
            if (!strcmp(instance_extension_properties[i].extensionName, extension)) {
                found = true;
            }
        }
        if (!found) {
            throw std::runtime_error("Missing required extension: "s + extension);
        }
    }

    const VkApplicationInfo application_info {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .apiVersion = VK_API_VERSION_1_3
    };
    const VkInstanceCreateInfo instance_create_info {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &application_info,
        .enabledExtensionCount = REQUIRED_INSTANCE_EXTENSIONS.size(),
        .ppEnabledExtensionNames = REQUIRED_INSTANCE_EXTENSIONS.data()
    };
    check_success(vkCreateInstance(&instance_create_info, nullptr, &d.instance));
    volkLoadInstanceOnly(d.instance);

    const VkWaylandSurfaceCreateInfoKHR surface_create_info {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .display = window.display(),
        .surface = window.surface()
    };
    check_success(vkCreateWaylandSurfaceKHR(d.instance, &surface_create_info, nullptr, &d.surface));

    const auto physical_device_info = select_physical_device(d.instance, d.surface);
    _physical_device = physical_device_info.physical_device;
    _queue_family_index = physical_device_info.graphics_queue;

    std::vector device_extensions{
        VK_KHR_MAINTENANCE_5_EXTENSION_NAME,
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    if (physical_device_info.has_memory_priority) {
        device_extensions.emplace_back(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME);
    }
    if (physical_device_info.has_pageable_device_local_memory) {
        device_extensions.emplace_back(VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME);
    }

    void *optional_pnext_chain = nullptr;

    VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT desired_pageable_memory_features {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PAGEABLE_DEVICE_LOCAL_MEMORY_FEATURES_EXT,
        .pNext = &desired_pageable_memory_features
    };
    if (physical_device_info.has_pageable_device_local_memory) {
        desired_pageable_memory_features.pageableDeviceLocalMemory = true;
        std::swap(optional_pnext_chain, desired_pageable_memory_features.pNext);
    }
    VkPhysicalDeviceMemoryPriorityFeaturesEXT desired_memory_priority_features {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT,
        .pNext = &desired_memory_priority_features
    };
    if (physical_device_info.has_memory_priority) {
        desired_memory_priority_features.memoryPriority = true;
        std::swap(optional_pnext_chain, desired_memory_priority_features.pNext);
    }
    const VkPhysicalDeviceMaintenance5FeaturesKHR desired_maintenance_5_features {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR,
        .pNext = optional_pnext_chain,
        .maintenance5 = true
    };
    const VkPhysicalDeviceVulkan13Features desired_vulkan_1_3_features {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = const_cast<VkPhysicalDeviceMaintenance5FeaturesKHR *>(&desired_maintenance_5_features),
        .synchronization2 = true
    };
    const float queue_priority = 1.0f;
    const VkDeviceQueueCreateInfo queue_create_info {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = _queue_family_index,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority
    };
    const VkDeviceCreateInfo device_create_info {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &desired_vulkan_1_3_features,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue_create_info,
        .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
        .ppEnabledExtensionNames = device_extensions.data()
    };
    check_success(vkCreateDevice(_physical_device, &device_create_info, nullptr, &d.device));
    volkLoadDevice(d.device);

    VmaAllocatorCreateFlags allocator_flags = VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT;
    if (physical_device_info.has_memory_priority) {
        allocator_flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
    }
    const VmaAllocatorCreateInfo allocator_create_info {
        .flags = allocator_flags,
        .physicalDevice = _physical_device,
        .device = d.device,
        .instance = d.instance,
        .vulkanApiVersion = application_info.apiVersion
    };
    check_success(vmaCreateAllocator(&allocator_create_info, &d.allocator));
    _swapchain.init(d.device, d.allocator, d.surface, _physical_device);

    vkGetDeviceQueue(d.device, _queue_family_index, 0, &_queue);

    const VkDescriptorSetLayoutBinding descriptor_set_layout_binding {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    };
    const VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &descriptor_set_layout_binding
    };
    check_success(vkCreateDescriptorSetLayout(d.device, &descriptor_set_layout_create_info, nullptr, &d.descriptor_set_layout));

    const VkPipelineLayoutCreateInfo pipeline_layout_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &d.descriptor_set_layout
    };
    check_success(vkCreatePipelineLayout(d.device, &pipeline_layout_create_info, nullptr, &d.pipeline_layout));

    const std::array attachment_descs {
        VkAttachmentDescription {
            .format = _swapchain.format(),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        },
        VkAttachmentDescription {
            .format = _swapchain.depth_format(),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL
        }
    };
    const VkAttachmentReference color_attachment_ref {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL
    };
    const VkAttachmentReference depth_attachment_ref {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL
    };
    const VkSubpassDescription subpass_desc {
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref,
        .pDepthStencilAttachment = &depth_attachment_ref
    };
    const std::array subpass_dependencies {
        VkSubpassDependency {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = VK_ACCESS_NONE,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
        },
        VkSubpassDependency {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
        }
    };
    const VkRenderPassCreateInfo render_pass_create_info {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = attachment_descs.size(),
        .pAttachments = attachment_descs.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass_desc,
        .dependencyCount = subpass_dependencies.size(),
        .pDependencies = subpass_dependencies.data()
    };
    check_success(vkCreateRenderPass(d.device, &render_pass_create_info, nullptr, &d.render_pass));

    const std::vector<uint32_t> vertex_code = load_shader("main.vert");
    const VkShaderModuleCreateInfo vertex_shader_create_info {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = vertex_code.size() * sizeof(uint32_t),
        .pCode = vertex_code.data()
    };
    const std::vector<uint32_t> fragment_code = load_shader("main.frag");
    const VkShaderModuleCreateInfo fragment_shader_create_info {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = fragment_code.size() * sizeof(uint32_t),
        .pCode = fragment_code.data()
    };
    const std::array pipeline_shader_stages {
        VkPipelineShaderStageCreateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = &vertex_shader_create_info,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .pName = "main"
        },
        VkPipelineShaderStageCreateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = &fragment_shader_create_info,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pName = "main"
        }
    };
    const std::array vertex_binding_descs {
        VkVertexInputBindingDescription { 
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        }
    };
    const std::array vertex_attribute_descs {
        VkVertexInputAttributeDescription {
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, position)
        },
        VkVertexInputAttributeDescription {
            .location = 1,
            .binding = 0,
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .offset = offsetof(Vertex, color)
        }
    };
    const VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = vertex_binding_descs.size(),
        .pVertexBindingDescriptions = vertex_binding_descs.data(),
        .vertexAttributeDescriptionCount = vertex_attribute_descs.size(),
        .pVertexAttributeDescriptions = vertex_attribute_descs.data()
    };
    const VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    };
    const VkPipelineViewportStateCreateInfo pipeline_viewport_state {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1
    };
    const VkPipelineRasterizationStateCreateInfo pipeline_raster_state {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .lineWidth = 1.0f  
    };
    const VkPipelineMultisampleStateCreateInfo pipeline_multisample_state {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };
    const VkPipelineDepthStencilStateCreateInfo pipeline_depth_stencil_state {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = true,
        .depthWriteEnable = true,
        .depthCompareOp = VK_COMPARE_OP_GREATER
    };
    const VkPipelineColorBlendAttachmentState pipeline_color_blend_attachment_state {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };
    const VkPipelineColorBlendStateCreateInfo pipeline_color_blend_state {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &pipeline_color_blend_attachment_state
    };
    const std::array pipeline_dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    const VkPipelineDynamicStateCreateInfo pipeline_dynamic_state {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = pipeline_dynamic_states.size(),
        .pDynamicStates = pipeline_dynamic_states.data()
    };
    const VkGraphicsPipelineCreateInfo pipeline_create_info {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = pipeline_shader_stages.size(),
        .pStages = pipeline_shader_stages.data(),
        .pVertexInputState = &pipeline_vertex_input_state,
        .pInputAssemblyState = &pipeline_input_assembly_state,
        .pViewportState = &pipeline_viewport_state,
        .pRasterizationState = &pipeline_raster_state,
        .pMultisampleState = &pipeline_multisample_state,
        .pDepthStencilState = &pipeline_depth_stencil_state,
        .pColorBlendState = &pipeline_color_blend_state,
        .pDynamicState = &pipeline_dynamic_state,
        .layout = d.pipeline_layout,
        .renderPass = d.render_pass
    };
    check_success(vkCreateGraphicsPipelines(d.device, nullptr, 1, &pipeline_create_info, nullptr, &d.pipeline)); // FIXME: PipelineCache

    const std::array descriptor_pool_sizes {
        VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1 }
    };
    const VkDescriptorPoolCreateInfo descriptor_pool_create_info {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,
        .poolSizeCount = descriptor_pool_sizes.size(),
        .pPoolSizes = descriptor_pool_sizes.data()
    };
    check_success(vkCreateDescriptorPool(d.device, &descriptor_pool_create_info, nullptr, &d.descriptor_pool));
    const VkDescriptorSetAllocateInfo descriptor_set_allocate_info {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = d.descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &d.descriptor_set_layout
    };
    check_success(vkAllocateDescriptorSets(d.device, &descriptor_set_allocate_info, &d.descriptor_set));

    const VkBufferCreateInfo index_buffer_create_info {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = sizeof(INDICES),
        .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT
    };
    const VmaAllocationCreateInfo staging_allocation_info {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, // FIXME: Change to staging-instead bit
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        .priority = HIGH_PRIORITY
    };
    vmaCreateBuffer(d.allocator, &index_buffer_create_info, &staging_allocation_info, &d.index_buffer, &d.index_allocation, nullptr);

    void *pData;
    vmaMapMemory(d.allocator, d.index_allocation, &pData);
    memcpy(pData, &INDICES, sizeof(INDICES));
    vmaUnmapMemory(d.allocator, d.index_allocation);

    const VkBufferCreateInfo vertex_buffer_create_info {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = sizeof(VERTICES),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    };
    vmaCreateBuffer(d.allocator, &vertex_buffer_create_info, &staging_allocation_info, &d.vertex_buffer, &d.vertex_allocation, nullptr);
    
    vmaMapMemory(d.allocator, d.vertex_allocation, &pData);
    memcpy(pData, &VERTICES, sizeof(VERTICES));
    vmaUnmapMemory(d.allocator, d.vertex_allocation);

    const VkBufferCreateInfo uniform_buffer_create_info {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = NUM_FRAMES_IN_FLIGHT * sizeof(MatrixUniforms),
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
    };
    const VmaAllocationCreateInfo mappable_allocation_info {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        .priority = NORMAL_PRIORITY
    };
    vmaCreateBuffer(d.allocator, &uniform_buffer_create_info, &mappable_allocation_info, &d.uniform_buffer, &d.uniform_allocation, nullptr);

    const VkDescriptorBufferInfo descriptor_buffer_info {
        .buffer = d.uniform_buffer,
        .offset = 0,
        .range = sizeof(MatrixUniforms)
    };
    const VkWriteDescriptorSet descriptor_write {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = d.descriptor_set,
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        .pBufferInfo = &descriptor_buffer_info
    };
    vkUpdateDescriptorSets(d.device, 1, &descriptor_write, 0, nullptr);

    for (auto& frame_data : d.frame_data) {
        const VkCommandPoolCreateInfo command_pool_create_info {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            .queueFamilyIndex = _queue_family_index
        };
        check_success(vkCreateCommandPool(d.device, &command_pool_create_info, nullptr, &frame_data.command_pool));

        const VkCommandBufferAllocateInfo command_buffer_allocate_info {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = frame_data.command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };
        check_success(vkAllocateCommandBuffers(d.device, &command_buffer_allocate_info, &frame_data.command_buffer));

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
}

Renderer::~Renderer() {
    if (d.device) {
        wait_all_fences();
        _swapchain.destroy(true);
    }
}

FrameData& Renderer::frame() noexcept {
    return d.frame_data[_frame_index];
}

void Renderer::render() {
    if (_swapchain.rebuild_required() || _window.size() != _swapchain.size()) {
        check_success(wait_all_fences()); // Can this be reduced?
        _swapchain.rebuild(_window.size(), d.render_pass);
    }

    _frame_index = (_frame_index + 1) % d.frame_data.size();
    check_success(vkWaitForFences(d.device, 1, &frame().fence, true, UINT64_MAX));
    if (_swapchain.acquire(frame().semaphore)) {
        record_command_buffer();

        const VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        const VkSubmitInfo submit_info {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &frame().semaphore,
            .pWaitDstStageMask = &wait_stage_mask,
            .commandBufferCount = 1,
            .pCommandBuffers = &frame().command_buffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &_swapchain.image_data().semaphore
        };
        check_success(vkResetFences(d.device, 1, &frame().fence));
        check_success(vkQueueSubmit(_queue, 1, &submit_info, frame().fence));

        _swapchain.present(_queue);
    }
}

void Renderer::record_command_buffer() {
    const VkCommandBufferBeginInfo command_buffer_begin_info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    const auto swapchain_size = _swapchain.size();
    const std::array clear_values {
        VkClearValue { .color = { .float32 = {0.0f, 0.0f, 0.0f, 0.0f} } },
        VkClearValue { .depthStencil = { .depth = 0.0f } }
    };
    const VkRenderPassBeginInfo render_pass_begin_info {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = d.render_pass,
        .framebuffer = _swapchain.image_data().framebuffer,
        .renderArea = { {0, 0}, swapchain_size },
        .clearValueCount = clear_values.size(),
        .pClearValues = clear_values.data()
    };

    const VkRect2D scissor = { {}, swapchain_size };
    const VkViewport viewport {
        .x = 0, .y = static_cast<float>(swapchain_size.height),
        .width = static_cast<float>(swapchain_size.width), .height = -static_cast<float>(swapchain_size.height),
        .minDepth = 0.0f, .maxDepth = 1.0f
    };

    const VkDeviceSize null_offset = 0;
    const auto matrix_uniforms_offset = static_cast<uint32_t>(_frame_index * sizeof(MatrixUniforms));

    const auto model = glm::translate(glm::vec3(-1.0f, -0.75f, 0.0f));
    const auto view = glm::lookAt(glm::vec3(0.0, 0.0, -2.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    const auto aspect = static_cast<float>(swapchain_size.width) / static_cast<float>(swapchain_size.height);
    const MatrixUniforms matrix_uniforms {
        .modelview = view * model,
        .projection = infinitePerspectiveFovReverse(FIELD_OF_VIEW, aspect, NEAR_CLIP_PLANE)
    };

    void *pData;
    vmaMapMemory(d.allocator, d.uniform_allocation, &pData);
    memcpy(&reinterpret_cast<MatrixUniforms *>(pData)[_frame_index], &matrix_uniforms, sizeof(MatrixUniforms));
    vmaUnmapMemory(d.allocator, d.uniform_allocation);

    check_success(vkResetCommandPool(d.device, frame().command_pool, 0));
    
    const auto cb = frame().command_buffer;
    check_success(vkBeginCommandBuffer(cb, &command_buffer_begin_info));
    vkCmdBeginRenderPass(cb, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, d.pipeline_layout, 0, 1, &d.descriptor_set, 1, &matrix_uniforms_offset);
    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, d.pipeline);
    vkCmdBindIndexBuffer(cb, d.index_buffer, null_offset, VK_INDEX_TYPE_UINT16);
    vkCmdBindVertexBuffers(cb, 0, 1, &d.vertex_buffer, &null_offset);
    vkCmdSetScissor(cb, 0, 1, &scissor);
    vkCmdSetViewport(cb, 0, 1, &viewport);
    vkCmdDrawIndexed(cb, 3, 1, 0, 0, 0);
    vkCmdEndRenderPass(cb);
    check_success(vkEndCommandBuffer(cb));
}
