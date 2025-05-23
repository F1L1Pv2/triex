#include <stdio.h>
#include <stdbool.h>

#define NOB_STRIP_PREFIX
#include "nob.h"

#include "vulkan/vulkan.h"

#include "vulkan_globals.h"
#include "vulkan_getDevice.h"

VkDevice device;
VkPhysicalDevice physicalDevice;
VkPhysicalDeviceLimits physicalDeviceLimits;
VkQueue graphicsQueue;
VkQueue presentQueue;
VkPhysicalDeviceMemoryProperties memoryProperties;
MultipleVkQueueFamilyProperties multipleQueueFamilyProperties;

typedef struct{
    VkPhysicalDevice* items;
    uint32_t count;
    size_t capacity;
} VkPhysicalDevices;

typedef struct{
    VkDeviceQueueCreateInfo* items;
    size_t count;
    size_t capacity;
} VkDeviceQueueCreateInfos;

typedef struct{
    float* items;
    size_t count;
    size_t capacity;
}   VkQueuePriorities;

typedef struct{
    VkQueuePriorities* items;
    size_t count;
    size_t capacity;
}   MultipleVkQueuePriorities;

bool getDevice(){

    VkPhysicalDevices physicalDevices = {0};

    vkEnumeratePhysicalDevices(instance,&physicalDevices.count,NULL);
    if(physicalDevices.count == 0) {
        printf("ERROR: No compatible GPU devices\n");
        return false;
    }

    da_resize(&physicalDevices,physicalDevices.count);
    vkEnumeratePhysicalDevices(instance,&physicalDevices.count,physicalDevices.items);

    physicalDevice = physicalDevices.items[0];

    if(physicalDevices.count > 1){
        for(size_t i = 0; i < physicalDevices.count; i++){
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(physicalDevices.items[i], &properties);

            if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                physicalDevice = physicalDevices.items[i];
                break;
            }
        }
    }

    VkPhysicalDeviceProperties physicalDeviceProperties = {0};
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    physicalDeviceLimits = physicalDeviceProperties.limits;
    
    printf("INFO: Chosen %s physical device\n", physicalDeviceProperties.deviceName);

    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,&multipleQueueFamilyProperties.count, NULL);
    da_resize(&multipleQueueFamilyProperties,multipleQueueFamilyProperties.count);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,&multipleQueueFamilyProperties.count, multipleQueueFamilyProperties.items);

    VkDeviceQueueCreateInfos queueCreateInfos = {0};

    MultipleVkQueuePriorities multipleVkQueuePriorities = {0};

    int graphicsQueueFamilyIndex = -1;
    int presentQueueFamilyIndex = -1;

    // finding needed queues
    for(size_t i = 0; i < multipleQueueFamilyProperties.count; i++){
        if(graphicsQueueFamilyIndex != -1 && presentQueueFamilyIndex != -1) break;

        if((multipleQueueFamilyProperties.items[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && graphicsQueueFamilyIndex == -1){
            graphicsQueueFamilyIndex = i;
        }

        if(presentQueueFamilyIndex == -1){
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
            if(presentSupport) presentQueueFamilyIndex = i;
        }
    }

    if(presentQueueFamilyIndex == -1){
        printf("ERROR: Couldn't find graphics queue\n");
        return false;
    }

    if(presentQueueFamilyIndex == -1){
        printf("ERROR: Couldn't find present queue\n");
        return false;
    }

    // Adding graphics queue familly into "Used families list"
    VkQueuePriorities vkQueuePriorities = {0};
    for(size_t j = 0; j < multipleQueueFamilyProperties.items[graphicsQueueFamilyIndex].queueCount; j++){
        da_append(&vkQueuePriorities, 1.0);
    }
    da_append(&multipleVkQueuePriorities, vkQueuePriorities);

    VkDeviceQueueCreateInfo queueCreateInfo = {0};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.pNext = NULL;
    queueCreateInfo.flags = 0;
    queueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
    queueCreateInfo.queueCount = multipleQueueFamilyProperties.items[graphicsQueueFamilyIndex].queueCount;
    queueCreateInfo.pQueuePriorities = multipleVkQueuePriorities.items[multipleVkQueuePriorities.count-1].items;
    da_append(&queueCreateInfos, queueCreateInfo);

    // If present queue familly is different from graphics queue one we repeat the process
    if(graphicsQueueFamilyIndex != presentQueueFamilyIndex){
        for(size_t j = 0; j < multipleQueueFamilyProperties.items[presentQueueFamilyIndex].queueCount; j++){
            da_append(&vkQueuePriorities, 1.0);
        }
        da_append(&multipleVkQueuePriorities, vkQueuePriorities);

        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.pNext = NULL;
        queueCreateInfo.flags = 0;
        queueCreateInfo.queueFamilyIndex = presentQueueFamilyIndex;
        queueCreateInfo.queueCount = multipleQueueFamilyProperties.items[presentQueueFamilyIndex].queueCount;
        queueCreateInfo.pQueuePriorities = multipleVkQueuePriorities.items[multipleVkQueuePriorities.count-1].items;
        da_append(&queueCreateInfos, queueCreateInfo);
    }

    if(queueCreateInfos.count == 0){
        printf("ERROR: Your physical device doesn't have any Of needed queues\n");
        return false;
    }

    VkDeviceCreateInfo deviceInfo = {0};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pNext = NULL;
    deviceInfo.flags = 0;
    deviceInfo.queueCreateInfoCount = queueCreateInfos.count;
    deviceInfo.pQueueCreateInfos = queueCreateInfos.items;
    const char* extensions[] = {
        "VK_KHR_swapchain"
    };
    deviceInfo.enabledExtensionCount = 1;
    deviceInfo.ppEnabledExtensionNames = extensions;
    deviceInfo.pEnabledFeatures = NULL;

    VkResult result = vkCreateDevice(physicalDevice,&deviceInfo,NULL,&device);
    if(result != VK_SUCCESS){
        printf("ERROR: Couldn't create device\n");
        return false;
    }

    for(size_t i = 0; i < multipleVkQueuePriorities.count; i++){
        da_free((multipleVkQueuePriorities.items[i]));
    }
    da_free(multipleVkQueuePriorities);
    da_free(queueCreateInfos);

    vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &graphicsQueue);
    vkGetDeviceQueue(device, presentQueueFamilyIndex, 0, &presentQueue);
    return true;
}