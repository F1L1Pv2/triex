#include <stdio.h>
#include <stdbool.h>

#define NOB_STRIP_PREFIX
#include "nob.h"

#include "platform_globals.h"

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#else
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#include "vulkan/vulkan.h"

#include "vulkan_globals.h"
#include "vulkan_createSurface.h"


VkSurfaceKHR surface;

bool createSurface(){

    #ifdef _WIN32
        VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfoKHR = {0};
        win32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        win32SurfaceCreateInfoKHR.pNext = NULL;
        win32SurfaceCreateInfoKHR.flags = 0;
        win32SurfaceCreateInfoKHR.hinstance = hInstance;
        win32SurfaceCreateInfoKHR.hwnd = hwnd;

        VkResult result = vkCreateWin32SurfaceKHR(instance, &win32SurfaceCreateInfoKHR, NULL, &surface);
    #else
        VkXlibSurfaceCreateInfoKHR xlibSurfaceCreateInfoKHR = {0};
        xlibSurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        xlibSurfaceCreateInfoKHR.pNext = NULL;
        xlibSurfaceCreateInfoKHR.flags = 0;
        xlibSurfaceCreateInfoKHR.dpy = display;
        xlibSurfaceCreateInfoKHR.window = window;

        VkResult result = vkCreateXlibSurfaceKHR(instance, &xlibSurfaceCreateInfoKHR, NULL, &surface);

    #endif
    if(result != VK_SUCCESS){
        printf("ERROR: Couldn't create window surface\n");
        return false;
    }

    return true;
}