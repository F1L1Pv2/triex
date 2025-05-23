#ifndef TRIEX_PLATFORM
#define TRIEX_PLATFORM

#include <stdbool.h>

void platform_create_window(const char* title, size_t width, size_t height);
void platform_window_handle_events();
bool platform_still_running();
void platform_sleep(size_t milis);

#endif