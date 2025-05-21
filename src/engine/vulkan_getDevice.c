#include <stdio.h>
#include <stdbool.h>

#define NOB_STRIP_PREFIX
#include "nob.h"

#include "vulkan/vulkan.h"

#include "vulkan_globals.h"
#include "vulkan_getDevice.h"

VkDevice device;

typedef struct{
    VkPhysicalDevice* items;
    uint32_t count;
    size_t capacity;
} VkPhysicalDevices;

typedef struct{
    VkQueueFamilyProperties* items;
    uint32_t count;
    size_t capacity;
} MultipleVkQueueFamilyProperties;

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

    VkPhysicalDevice physicalDevice;

    if(physicalDevices.count > 1){
        TODO("handle multiple devices");
    }else{
        physicalDevice = physicalDevices.items[0];
    }

    VkPhysicalDeviceProperties physicalDeviceProperties = {0};
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    printf("INFO: Chose %s physical device\n", physicalDeviceProperties.deviceName);

    MultipleVkQueueFamilyProperties multipleQueueFamilyProperties = {0};
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,&multipleQueueFamilyProperties.count, NULL);
    da_resize(&multipleQueueFamilyProperties,multipleQueueFamilyProperties.count);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,&multipleQueueFamilyProperties.count, multipleQueueFamilyProperties.items);

    VkDeviceQueueCreateInfos queueCreateInfos = {0};

    MultipleVkQueuePriorities multipleVkQueuePriorities = {0};

    for(size_t i = 0; i < multipleQueueFamilyProperties.count; i++){
        VkQueueFamilyProperties queueFamilyProperties = multipleQueueFamilyProperties.items[i];

        if((queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) || (queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT)){
            VkQueuePriorities vkQueuePriorities = {0};
            for(size_t j = 0; j < queueFamilyProperties.queueCount; j++){
                da_append(&vkQueuePriorities, 1.0);
            }
            da_append(&multipleVkQueuePriorities, vkQueuePriorities);

            VkDeviceQueueCreateInfo queueCreateInfo = {0};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.pNext = NULL;
            queueCreateInfo.flags = 0;
            queueCreateInfo.queueFamilyIndex = i;
            queueCreateInfo.queueCount = queueFamilyProperties.queueCount;
            queueCreateInfo.pQueuePriorities = multipleVkQueuePriorities.items[multipleVkQueuePriorities.count-1].items;

            da_append(&queueCreateInfos, queueCreateInfo);
        }
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

    return true;
}