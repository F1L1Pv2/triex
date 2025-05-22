#include <stdio.h>
#include <stdbool.h>

#define NOB_STRIP_PREFIX
#include "nob.h"

#include "vulkan/vulkan.h"

#include "vulkan_globals.h"
#include "vulkan_synchro.h"

VkSemaphore imageAvailableSemaphore;
VkSemaphore renderFinishedSemaphore;
VkFence inFlightFence;

bool createAllNeededSyncrhonizationObjects(){
    VkSemaphoreCreateInfo semaphoreInfo = {0};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkResult result;

    result = vkCreateSemaphore(device, &semaphoreInfo, NULL, &imageAvailableSemaphore);
    
    if(result != VK_SUCCESS){
        printf("ERROR: Couldn't imageAvailableSemaphore\n");
        return false;
    }

    result = vkCreateSemaphore(device, &semaphoreInfo, NULL, &renderFinishedSemaphore);
    
    if(result != VK_SUCCESS){
        printf("ERROR: Couldn't renderFinishedSemaphore\n");
        return false;
    }

    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    result = vkCreateFence(device, &fenceInfo, NULL, &inFlightFence);
    
    if(result != VK_SUCCESS){
        printf("ERROR: Couldn't inFlightFence\n");
        return false;
    }

    return true;
}