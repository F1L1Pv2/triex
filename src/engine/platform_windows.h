#include "MinWin.h"
#include <stdbool.h>
#include <malloc.h>

#include "platform.h"
#include "platform_globals.h"

static bool window_open = false;

bool platform_still_running(){
    return window_open;
}

LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg){
        case WM_CLOSE: window_open = false; break;
    }
    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

HINSTANCE hInstance;
HWND hwnd;

void platform_create_window(const char* title, size_t width, size_t height){
    RECT wr;
    wr.left = 100;
    wr.right = width + wr.left;
    wr.top = 100;
    wr.bottom = height + wr.top;
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, false);

    hInstance = GetModuleHandleA(NULL);

    WNDCLASSEXA windowClass = {0};
    windowClass.cbSize = sizeof(WNDCLASSEXA);
    windowClass.style = 0;
    windowClass.lpfnWndProc = HandleMsg;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = hInstance;
    windowClass.hIcon = NULL;
    windowClass.hCursor = NULL;
    windowClass.hbrBackground = NULL;
    windowClass.lpszMenuName = "TRIEX";
    windowClass.lpszClassName = "TRIEX";
    windowClass.hIconSm = NULL;
    RegisterClassExA(&windowClass);

    hwnd = CreateWindowExA(
        0,
        "TRIEX",
        title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, SW_SHOW);
    window_open = true;
}

MSG msg;

void platform_window_handle_events(){
    while(PeekMessageA(&msg,NULL,0,0,PM_REMOVE)){
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

void platform_sleep(size_t milis){
    Sleep(milis);
}