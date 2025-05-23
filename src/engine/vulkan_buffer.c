#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "vulkan/vulkan.h"

#include "vulkan_globals.h"
#include "vulkan_buffer.h"
#include "vulkan_helpers.h"

bool createBuffer(VkBufferUsageFlagBits usage, VkMemoryPropertyFlagBits memoryProperties, VkDeviceSize size, VkBuffer* outBuff, VkDeviceMemory* outMemory){
    VkResult result;
    
    VkBufferCreateInfo vertexBufferCreateInfo = {0};
    vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertexBufferCreateInfo.pNext = NULL;
    vertexBufferCreateInfo.flags = 0;
    vertexBufferCreateInfo.size = size;
    vertexBufferCreateInfo.usage = usage;
    vertexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vertexBufferCreateInfo.queueFamilyIndexCount = 0;
    vertexBufferCreateInfo.pQueueFamilyIndices = NULL;
    result = vkCreateBuffer(device, &vertexBufferCreateInfo, NULL, outBuff);
    if(result != VK_SUCCESS){
        printf("ERROR: Couldn't create buffer\n");
        return false;
    }

    VkMemoryRequirements vertexMemRequirements;
    vkGetBufferMemoryRequirements(device, *outBuff, &vertexMemRequirements);
    VkMemoryAllocateInfo vertexMemoryAllocateInfo = {0};
    vertexMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vertexMemoryAllocateInfo.pNext = NULL;
    vertexMemoryAllocateInfo.allocationSize = vertexMemRequirements.size;
    uint32_t index;
    if(!findMemoryType(vertexMemRequirements.memoryTypeBits, memoryProperties, &index)){
        printf("ERROR: Couldn't find memory type\n");
        return false;
    }
    vertexMemoryAllocateInfo.memoryTypeIndex = index;
    result = vkAllocateMemory(device, &vertexMemoryAllocateInfo,NULL,outMemory);
    if(result != VK_SUCCESS){
        printf("ERROR: Couldn't allocate memory\n");
        return false;
    }

    result = vkBindBufferMemory(device, *outBuff, *outMemory, 0);
    if(result != VK_SUCCESS){
        printf("ERROR: Couldn't bind memory to a buffer\n");
        return false;
    }

    return true;
}

bool transferDataToMemory(VkDeviceMemory memory, void* data, size_t offset, size_t size){
    void *mapped;
    VkResult result = vkMapMemory(device, memory, offset, size, 0, &mapped);
    if(result != VK_SUCCESS){
        printf("ERROR: Couldn't map memory\n");
        return false;
    }
    memcpy(mapped,data,size);
    vkUnmapMemory(device, memory);

    return true;
}