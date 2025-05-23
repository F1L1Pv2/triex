#ifndef TRIEX_VULKAN_HELPERS
#define TRIEX_VULKAN_HELPERS

#include <stdbool.h>

#include "vulkan/vulkan.h"
int getNeededQueueFamilyIndex(VkQueueFlags flags);
bool findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t* index);

#endif