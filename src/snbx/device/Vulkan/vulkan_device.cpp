#include "snbx/device/gpu_device_api.hpp"

#include "volk.h"
#include "vk_mem_alloc.h"
#include "vulkan_platform.hpp"

struct VulkanContext {
    VkInstance  instance = nullptr;
    VkDevice    device = nullptr;
    VkDebugUtilsMessengerEXT        debug_messenger = nullptr;
    VkPhysicalDevice                physical_device{};
    VkPhysicalDeviceProperties      gpu_properties{};
    VmaAllocator                    allocator{};
    VkCommandPool                   command_pool{};
    VkPhysicalDeviceFeatures        device_features{};
    GPUDeviceInfo                   device_info{};
    VkDescriptorPool                descriptor_pool{};

    //queues
    Vec<VkQueueFamilyProperties>    queue_families{};

    u32                             graphics_family{U32_MAX};
    u32                             present_family{U32_MAX};
    VkQueue                         graphics_queue{};
    VkQueue                         present_queue{};

    //raytrace
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR         ray_tracing_pipeline_properties{};
    VkPhysicalDeviceAccelerationStructurePropertiesKHR      acceleration_structure_properties_properties{};

    //Validation layers
    bool        enable_validation_layers      = true;
    const char* validation_layers[1]         = {"VK_LAYER_KHRONOS_validation"};
    bool        validation_layers_available   = false;
} context;

#define SNBX_CHECK_RESULT(func) result = func; \
    if (result != GPUResult::Success) return result


inline VkBool32 VKAPI_PTR vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                         VkDebugUtilsMessageTypeFlagsEXT message_type,
                                         const VkDebugUtilsMessengerCallbackDataEXT *callback_data_ext,
                                         void *user_data) {
    switch (message_severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            spdlog::trace("{}", callback_data_ext->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            spdlog::info("{}", callback_data_ext->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            spdlog::warn("{}", callback_data_ext->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            spdlog::error("{}", callback_data_ext->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
            break;
    }
    return VK_FALSE;
}

bool vk_query_instance_extensions(const Span<const char *> &required_extensions) {
    u32 extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    Vec<VkExtensionProperties> extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

    for (const auto req_extension: required_extensions) {
        bool found = false;
        for (const auto &extension: extensions) {
            if (strcmp(extension.extensionName, req_extension) == 0) {
                found = true;
            }
        }

        if (!found) {
            return false;
        }
    }
    return true;
}

bool vk_query_instance_extension(const char *extension) {
    return vk_query_instance_extensions(Span<const char *>(&extension, &extension + 1));
}

void vk_populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info)
{
    create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = &vk_debug_callback;
}


VkResult vk_create_debug_utils_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *create_info, const VkAllocationCallbacks *allocator, VkDebugUtilsMessengerEXT *debug_messenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, create_info, allocator, debug_messenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks *allocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debug_messenger, allocator);
    }
}

bool vk_query_layer_properties(const char **required_layers, u32 size) {
    u32 extension_count = 0;
    vkEnumerateInstanceLayerProperties(&extension_count, nullptr);
    Vec<VkLayerProperties> extensions(extension_count);
    vkEnumerateInstanceLayerProperties(&extension_count, extensions.data());

    for (int i = 0; i < size; ++i) {
        const auto req_extension = required_layers[i];
        bool found = false;
        for (const auto &layer: extensions) {
            if (strcmp(layer.layerName, req_extension) == 0) {
                found = true;
            }
        }

        if (!found) {
            return false;
        }
    }
    return true;
}

bool vk_query_device_extensions(const Span<VkExtensionProperties>& extensions, const char* check_for_extension) {
    for (const auto& extension: extensions) {
        if (strcmp(extension.extensionName, check_for_extension) == 0) {
            return true;
        }
    }
    return false;
}

GPUResult vk_create_instance() {
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "SNBX";
    app_info.applicationVersion = 0;
    app_info.pEngineName = "SNBX";
    app_info.engineVersion = 0;
    app_info.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;


    u32 validation_layer_size = sizeof(context.validation_layers) / sizeof(const char*);
    context.validation_layers_available = (context.enable_validation_layers && vk_query_layer_properties(context.validation_layers, validation_layer_size));

    VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info{};

    if (context.validation_layers_available)
    {
        create_info.enabledLayerCount = validation_layer_size;
        create_info.ppEnabledLayerNames = context.validation_layers;
        vk_populate_debug_messenger_create_info(debug_utils_messenger_create_info);
        create_info.pNext = &debug_utils_messenger_create_info;
    }
    else
    {
        create_info.enabledLayerCount = 0;
    }

   auto platform_req = platform_get_required_instance_extensions();

    Vec<const char*> required_extensions{};
    required_extensions.insert(required_extensions.begin(), platform_req.begin(), platform_req.end());
    if (context.validation_layers_available) {
        required_extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    if (!vk_query_instance_extensions(required_extensions)) {
        spdlog::error("[Vulkan] Required extensions not found");
        return GPUResult::NotSupported;
    }

#ifdef SNBX_MACOS
    if (query_instance_extensions(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
        required_extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
#endif

    create_info.enabledExtensionCount = required_extensions.size();
    create_info.ppEnabledExtensionNames = required_extensions.data();

    if (vkCreateInstance(&create_info, nullptr, &context.instance) != VK_SUCCESS) {
        spdlog::error("[Vulkan] error on create vkCreateInstance");
        return GPUResult::NotSupported;
    }

    return GPUResult::Success;
}

void setup_debug_messenger() {
    if (context.validation_layers_available) {
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        vk_populate_debug_messenger_create_info(createInfo);
        if (vk_create_debug_utils_messenger_ext(context.instance, &createInfo, nullptr, &context.debug_messenger) != VK_SUCCESS) {
            spdlog::warn("[Vulkan] failed to set up debug messenger!");
        }
    }
}

VkPhysicalDevice vk_choose_physical_device(const Vec<VkPhysicalDevice>& devices) {
    if (devices.size() == 1) {
        return devices[0];
    }

    // if (context.renderSettings->gpuIndex != U32_MAX && devices.Size() > context.renderSettings->gpuIndex)
    // {
    //     return devices[context.renderSettings->gpuIndex];
    // }

    Vec<std::pair<u32, VkPhysicalDevice>> device_score{};
    device_score.reserve(devices.size());

    for (VkPhysicalDevice device: devices) {
        u32 score = 0;

        VkPhysicalDeviceProperties device_properties;
        VkPhysicalDeviceFeatures device_features;
        vkGetPhysicalDeviceProperties(device, &device_properties);
        vkGetPhysicalDeviceFeatures(device, &device_features);

        if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 10000;
        }

        score += device_properties.limits.maxImageDimension2D;
        device_score.emplace_back(score, device);
        spdlog::debug("[Vulkan] device {} found", device_properties.deviceName);
    }

    std::ranges::sort(device_score, [](const std::pair<u32, VkPhysicalDevice>& r, const std::pair<u32, VkPhysicalDevice>& l) {
        return r.first > l.first;
    });
    return device_score[0].second;
}

GPUResult vk_select_physical_device() {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(context.instance, &device_count, nullptr);

    if (device_count == 0) {
        spdlog::error("failed to find GPUs with Vulkan support!");
        return GPUResult::NotSupported;
    }

    Vec<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(context.instance, &device_count, devices.data());

    context.physical_device = vk_choose_physical_device(devices);

    vkGetPhysicalDeviceFeatures(context.physical_device, &context.device_features);
    context.device_info.multi_draw_indirect_supported = context.device_features.multiDrawIndirect;

    VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES, nullptr};
    VkPhysicalDeviceFeatures2 deviceFeatures2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &indexingFeatures};
    vkGetPhysicalDeviceFeatures2(context.physical_device, &deviceFeatures2);
    context.device_info.bindless_supported = indexingFeatures.descriptorBindingPartiallyBound && indexingFeatures.runtimeDescriptorArray;

    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(context.physical_device, nullptr, &extension_count, nullptr);

    Vec<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(context.physical_device, nullptr, &extension_count, available_extensions.data());

    for (int i = 0; i < extension_count; ++i) {
        spdlog::debug("[Vulkan] Extension {} ", available_extensions[i].extensionName);
    }

    if (!vk_query_device_extensions(available_extensions,VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
        spdlog::warn("[Vulkan] extension {} not found", VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        return GPUResult::NotSupported;
    }

    //vulkanDeviceInfo.dynamicRenderingExtensionPresent = vk_query_device_extensions(availableExtensions, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    //context.vulkanDeviceInfo.maintenance4Available = vk_query_device_extensions(availableExtensions, VK_KHR_MAINTENANCE_4_EXTENSION_NAME);

    context.device_info.raytrace_supported = vk_query_device_extensions(available_extensions, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    spdlog::debug("[Vulkan] Raytrace enabled");

    vkGetPhysicalDeviceProperties(context.physical_device, &context.gpu_properties);

    if (context.device_info.raytrace_supported) {
        context.ray_tracing_pipeline_properties = {};
        context.ray_tracing_pipeline_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

        VkPhysicalDeviceProperties2 device_properties2{};
        device_properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        device_properties2.pNext = &context.ray_tracing_pipeline_properties;
        vkGetPhysicalDeviceProperties2(context.physical_device, &device_properties2);

        context.acceleration_structure_properties_properties = {};
        context.acceleration_structure_properties_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;

        VkPhysicalDeviceFeatures2 features2{};
        features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features2.pNext = &context.acceleration_structure_properties_properties;
        vkGetPhysicalDeviceFeatures2(context.physical_device, &features2);
    }
    return GPUResult::Success;
}

void vk_update_required_queue_families() {
    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(context.physical_device, &queueFamilyCount, nullptr);
    context.queue_families.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(context.physical_device, &queueFamilyCount, context.queue_families.data());

    int i = 0;
    for (const auto& queue_family: context.queue_families) {
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            context.graphics_family = i;
        }

        //try to keep graphics and present in the same queue
        if (platform_get_physical_device_presentation_support(context.instance, context.physical_device, i)) {
            context.present_family = i;
        }

        if (context.graphics_family != U32_MAX && context.present_family != U32_MAX) {
            break;
        }
        i++;
    }
}

GPUResult vk_create_logical_device() {
    f32 queuePriority = 1.0f;

    //**raytrace***
    VkPhysicalDeviceRayQueryFeaturesKHR device_ray_query_features_khr{};
    device_ray_query_features_khr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
    device_ray_query_features_khr.pNext = nullptr;
    device_ray_query_features_khr.rayQuery = VK_TRUE;

    VkPhysicalDeviceAccelerationStructureFeaturesKHR device_acceleration_structure_features_khr{};

    device_acceleration_structure_features_khr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    device_acceleration_structure_features_khr.pNext = &device_ray_query_features_khr;
    device_acceleration_structure_features_khr.accelerationStructure = VK_TRUE;

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR deviceRayTracingPipelineFeatures{};
    deviceRayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    deviceRayTracingPipelineFeatures.pNext = &device_acceleration_structure_features_khr;
    deviceRayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;

    //endraytrace.
    Vec<VkDeviceQueueCreateInfo> queue_create_infos{};
    if (context.graphics_family != context.present_family) {
        queue_create_infos.resize(2);

        queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[0].queueFamilyIndex = context.graphics_family;
        queue_create_infos[0].queueCount = 1;
        queue_create_infos[0].pQueuePriorities = &queuePriority;

        queue_create_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[1].queueFamilyIndex = context.present_family;
        queue_create_infos[1].queueCount = 1;
        queue_create_infos[1].pQueuePriorities = &queuePriority;
    } else {
        queue_create_infos.resize(1);
        queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[0].queueFamilyIndex = context.graphics_family;
        queue_create_infos[0].queueCount = 1;
        queue_create_infos[0].pQueuePriorities = &queuePriority;
    }
    VkPhysicalDeviceFeatures physical_device_features{};

    if (context.device_features.samplerAnisotropy) {
        physical_device_features.samplerAnisotropy = VK_TRUE;
    }

    if (context.device_info.multi_draw_indirect_supported) {
        physical_device_features.multiDrawIndirect = VK_TRUE;
    }

    physical_device_features.shaderInt64 = VK_TRUE;
    physical_device_features.fillModeNonSolid = true;

    VkPhysicalDeviceVulkan12Features features12{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    features12.bufferDeviceAddress = VK_TRUE;


    VkPhysicalDeviceMaintenance4FeaturesKHR vk_physical_device_maintenance4_features_khr{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES_KHR};
    vk_physical_device_maintenance4_features_khr.maintenance4 = true;

    if (context.device_info.raytrace_supported) {
        vk_physical_device_maintenance4_features_khr.pNext = &deviceRayTracingPipelineFeatures;
    }

    // if (context.vulkanDeviceInfo.maintenance4Available) {
    // 	features12.pNext = &vkPhysicalDeviceMaintenance4FeaturesKhr;
    // }

    if (context.device_info.bindless_supported) {
        features12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        features12.runtimeDescriptorArray = VK_TRUE;
        features12.descriptorBindingVariableDescriptorCount = VK_TRUE;
        features12.descriptorBindingPartiallyBound = VK_TRUE;
        features12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
        features12.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
    }

    VkDeviceCreateInfo create_info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.queueCreateInfoCount = queue_create_infos.size();
    create_info.pEnabledFeatures = &physical_device_features;

    Vec<const char *> deviceExtensions{};
    deviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);


    // if (context.vulkanDeviceInfo.maintenance4Available) {
    // 	deviceExtensions.emplace_back(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    // }

    if (context.device_info.raytrace_supported) {
        deviceExtensions.emplace_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
        deviceExtensions.emplace_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
        deviceExtensions.emplace_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
        deviceExtensions.emplace_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
        deviceExtensions.emplace_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
        deviceExtensions.emplace_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
        deviceExtensions.emplace_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
        deviceExtensions.emplace_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    }

    // if (context.vulkanDeviceInfo.dynamicRenderingExtensionPresent) {
    // 	deviceExtensions.emplace_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    // }

#ifdef SNBX_MACOS
		deviceExtensions.emplace_back("VK_KHR_portability_subset");
#endif

    create_info.enabledExtensionCount = static_cast<u32>(deviceExtensions.size());
    create_info.ppEnabledExtensionNames = deviceExtensions.data();

    if (context.validation_layers_available) {
        auto validationLayerSize = sizeof(context.validation_layers) / sizeof(const char *);
        create_info.enabledLayerCount = validationLayerSize;
        create_info.ppEnabledLayerNames = context.validation_layers;
    } else {
        create_info.enabledLayerCount = 0;
    }

    // VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR};
    // if (context.vulkanDeviceInfo.dynamicRenderingExtensionPresent) {
    //     dynamicRenderingFeatures.dynamicRendering = true;
    //     dynamicRenderingFeatures.pNext = features12.pNext;
    //     features12.pNext = &dynamicRenderingFeatures;
    //     context.logger.Debug("Dynamic Rendering enabled");
    // }

    create_info.pNext = &features12;
    if (vkCreateDevice(context.physical_device, &create_info, nullptr, &context.device) != VK_SUCCESS) {
        spdlog::error("failed to create device");
        return GPUResult::NotSupported;
    }

    return GPUResult::Success;
}


GPUResult vk_init() {
    GPUResult result;

    platform_init_vk();
    volkInitialize();
    SNBX_CHECK_RESULT(vk_create_instance());
    setup_debug_messenger();
    volkLoadInstance(context.instance);
    SNBX_CHECK_RESULT(vk_select_physical_device());
    vk_update_required_queue_families();
    SNBX_CHECK_RESULT(vk_create_logical_device());

    spdlog::info("Vulkan API {}.{}.{} Device: {} ",
                 VK_VERSION_MAJOR(context.gpu_properties.apiVersion),
                 VK_VERSION_MINOR(context.gpu_properties.apiVersion),
                 VK_VERSION_PATCH(context.gpu_properties.apiVersion),
                 context.gpu_properties.deviceName);

    return result;
}

GPUSwapchain vk_create_swapchain(const SwapchainCreation& swapchain_creation, Window* window) {
    return {};
}

void vk_destroy_swapchain(const GPUSwapchain& swapchain) {

}

void vk_shutdown() {
    vkDestroyDevice(context.device, nullptr);
    destroy_debug_utils_messenger_ext(context.instance, context.debug_messenger, nullptr);
    vkDestroyInstance(context.instance, nullptr);
}

void vulkan_device_register(GPUDeviceAPI& gpu_device_api) {
    gpu_device_api.init = vk_init;
    gpu_device_api.shutdown = vk_shutdown;
    gpu_device_api.create_swapchain = vk_create_swapchain;
    gpu_device_api.destroy_swapchain = vk_destroy_swapchain;
}