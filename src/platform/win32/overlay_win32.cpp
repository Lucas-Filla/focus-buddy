#include "../overlay.h"
#include <windows.h>

void makeColorKeyTransparent(void* windowHandle, unsigned char r, unsigned char g, unsigned char b) {
    HWND hwnd = static_cast<HWND>(windowHandle);
    LONG_PTR exStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    SetWindowLongPtrW(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, RGB(r, g, b), 0, LWA_COLORKEY);
}