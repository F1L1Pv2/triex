#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "vulkan/vulkan.h"

#include "vulkan_globals.h"
#include "vulkan_helpers.h"

int getNeededQueueFamilyIndex(VkQueueFlags flags){
    for(int i = 0; i < multipleQueueFamilyProperties.count; i++){
        if(multipleQueueFamilyProperties.items[i].queueFlags & flags){
            return i;
        }
    }
    return -1;
}

bool findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t* index) {
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            *index = i;
            return true;
        }
    }
    return false;
}