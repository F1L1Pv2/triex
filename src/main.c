#include <stdio.h>
#include <stdint.h>

#define NOB_STRIP_PREFIX
#include "nob.h"

#include "engine/vulkan_globals.h"
#include "engine/vulkan_initialize.h"
#include "engine/vulkan_getDevice.h"

typedef struct {
    float x;
    float y;
} Vertex;

int main(){
    printf("hello from C\nVULKANO PREPARE!\n");

    if(!initialize_vulkan()) return 1;
    if(!getDevice()) return 1;

    printf("Created Instance %p\n", instance);
    return 0;
}