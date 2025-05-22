#include <stdio.h>
#include <stdbool.h>

#define NOB_STRIP_PREFIX
#include "nob.h"

#include "vulkan/vulkan.h"

#include "vulkan_globals.h"
#include "vulkan_initCommandPool.h"
#include "vulkan_helpers.h"

VkCommandPool commandPool;

bool initCommandPool(){
    
    VkCommandPoolCreateInfo commandPoolInfo = {0};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.pNext = NULL;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    int index = getNeededQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
    if(index < 0){
        printf("ERROR: Couldn't get desired queue family\n");
        return false;
    }
    commandPoolInfo.queueFamilyIndex = index;
    VkResult result = vkCreateCommandPool(device,&commandPoolInfo,NULL,&commandPool);
    if(result != VK_SUCCESS){
        printf("ERROR: Couldn't create command pool");
        return false;
    }

    return true;
}