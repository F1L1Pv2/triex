#include <X11/X.h>
#define XK_LATIN1
#define XK_MISCELLANY
#include <X11/keysymdef.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrandr.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/stat.h>

#include "platform.h"
#include "platform_globals.h"

bool running = false;
static Atom wmDeleteWindow;

Window window;
Display* display;

void platform_create_window(const char* title, size_t width, size_t height){
    display = XOpenDisplay(NULL);

    window = XCreateSimpleWindow(display, 
                               DefaultRootWindow(display),
                               10,      // xPos
                               10,      // yPos
                               width, 
                               height,
                               0,       // border width
                               0,       // border
                               0);       // background

  // Set the input mask for our window on the current display
  // ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | 
  // PointerMotionMask | ButtonMotionMask | FocusChangeMask
  long event_mask = ExposureMask
                | KeyPressMask | KeyReleaseMask
                | ButtonPressMask | ButtonReleaseMask
                | StructureNotifyMask;
  XSelectInput(display, window, event_mask);

  XMapWindow(display, window);

  // Tell the server to notify us when the window manager attempts to destroy the window
  wmDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &wmDeleteWindow, 1);

  running = true;

  XStoreName(display, window, title);
}

void platform_window_handle_events() {
    while (XPending(display)) {
        XEvent event;
        XNextEvent(display, &event);

        switch(event.type) {
            case ClientMessage: {
                Atom wmProtocol = XInternAtom(display, "WM_PROTOCOLS", False);
                Atom wmDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", False);

                if (event.xclient.message_type == wmProtocol &&
                    event.xclient.data.l[0] == wmDeleteWindow) {
                    running = false;
                }
                break;
            }
        }
    }
}

bool platform_still_running(){
    return running;
}

void platform_sleep(size_t milis){
    usleep(milis);
}