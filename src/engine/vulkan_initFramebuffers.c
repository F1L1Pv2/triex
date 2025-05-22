#include <stdio.h>
#include <stdbool.h>

#define NOB_STRIP_PREFIX
#include "nob.h"

#include "vulkan/vulkan.h"

#include "vulkan_globals.h"
#include "vulkan_initFramebuffers.h"

VkFramebuffers framebuffers;

bool initFramebuffers(){
    da_resize(&framebuffers,swapchainImages.count);

    if(framebuffers.count == 0){
        printf("ERROR: Couldn't create framebuffers\n");
        return false;
    }

    VkResult result;

    for(int i = 0; i < framebuffers.count; i++){
        VkFramebufferCreateInfo framebufferCreateInfo = {0};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.pNext = NULL;
        framebufferCreateInfo.flags = 0;
        framebufferCreateInfo.renderPass = renderPass;
        framebufferCreateInfo.attachmentCount = 1;
        framebufferCreateInfo.pAttachments = &swapchainImageViews.items[i];
        framebufferCreateInfo.width = swapchainExtent.width;
        framebufferCreateInfo.height = swapchainExtent.height;
        framebufferCreateInfo.layers = 1;

        result = vkCreateFramebuffer(device, &framebufferCreateInfo,NULL,&framebuffers.items[i]);
        if(result != VK_SUCCESS){
            printf("ERROR: Couldn't create framebuffer\n");
            return false;
        }
    }

    return true;
}