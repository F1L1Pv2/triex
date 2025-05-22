#ifndef FVFX_PLATFORM_TYPES
#define FVFX_PLATFORM_TYPES

#ifdef _WIN32
#include "MinWin.h"

extern HWND hwnd;
extern HINSTANCE hInstance;

#else

#include <X11/X.h>
#include <X11/Xlib.h>

extern Display* display;
extern Window window;

#endif

#endif