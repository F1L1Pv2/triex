#ifndef TRIEX_VULKAN_BUFFER
#define TRIEX_VULKAN_BUFFER

#include <stdbool.h>
#include <vulkan/vulkan.h>

bool createBuffer(VkBufferUsageFlagBits usage, VkMemoryPropertyFlagBits memoryProperties, VkDeviceSize size, VkBuffer* outBuff, VkDeviceMemory* outMemory);
bool transferDataToMemory(VkDeviceMemory memory, void* data, size_t offset, size_t size);

#endif