#include <stdio.h>
#include <stdbool.h>

#define NOB_STRIP_PREFIX
#include "nob.h"

#include "vulkan/vulkan.h"

#include "vulkan_globals.h"
#include "vulkan_initCommandBuffer.h"

VkCommandBuffer cmd;

bool initCommandBuffer(){
    VkCommandBufferAllocateInfo cmdInfo = {0};
    cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdInfo.pNext = NULL;
    cmdInfo.commandPool = commandPool;
    cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdInfo.commandBufferCount = 1;
    VkResult result = vkAllocateCommandBuffers(device,&cmdInfo,&cmd);
    if(result != VK_SUCCESS){
        printf("ERROR: Couldn't allocate command buffer\n");
        return false;
    }    

    return true;
}