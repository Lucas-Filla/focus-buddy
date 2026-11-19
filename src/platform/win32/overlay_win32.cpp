#include "../overlay.h"

#include <windows.h>
#include <cstdint>

namespace {

    HWND    g_window = nullptr;
    HDC     g_memoryDC = nullptr;
    HBITMAP g_bitmap = nullptr;
    void* g_pixels = nullptr;
    int     g_width = 0;
    int     g_height = 0;
    bool    g_closeWanted = false;

    const wchar_t* kClassName = L"FocusBuddyOverlay";

    LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
        case WM_NCHITTEST:
            return HTCAPTION;

        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                g_closeWanted = true;
                return 0;
            }
            break;

        case WM_CLOSE:
            g_closeWanted = true;
            return 0;
        }
        return DefWindowProcW(window, message, wParam, lParam);
    }

} 

namespace overlay {

    bool create(int width, int height) {
        g_width = width;
        g_height = height;
        HINSTANCE instance = GetModuleHandleW(nullptr);

        WNDCLASSEXW windowClass{};
        windowClass.cbSize = sizeof(windowClass);
        windowClass.lpfnWndProc = windowProc;
        windowClass.hInstance = instance;
        windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
        windowClass.lpszClassName = kClassName;
        if (!RegisterClassExW(&windowClass)) return false;

        const int startX = 200, startY = 200;
        g_window = CreateWindowExW(WS_EX_LAYERED | WS_EX_TOPMOST, kClassName, L"Focus Buddy",
            WS_POPUP, startX, startY, width, height,
            nullptr, nullptr, instance, nullptr);
        if (!g_window) return false;

        BITMAPINFO info{};
        info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth = width;
        info.bmiHeader.biHeight = -height;  
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 32;
        info.bmiHeader.biCompression = BI_RGB;

        HDC screen = GetDC(nullptr);
        g_memoryDC = CreateCompatibleDC(screen);
        ReleaseDC(nullptr, screen);
        g_bitmap = CreateDIBSection(g_memoryDC, &info, DIB_RGB_COLORS, &g_pixels, nullptr, 0);
        if (!g_bitmap) return false;
        SelectObject(g_memoryDC, g_bitmap);

        ShowWindow(g_window, SW_SHOW);
        return true;
    }

    void present(const unsigned char* rgba, int width, int height) {
        if (!g_window || width != g_width || height != g_height) return;

        auto* out = static_cast<std::uint8_t*>(g_pixels);
        const int count = width * height;
        for (int i = 0; i < count; ++i) {
            const std::uint8_t r = rgba[i * 4 + 0];
            const std::uint8_t g = rgba[i * 4 + 1];
            const std::uint8_t b = rgba[i * 4 + 2];
            const std::uint8_t a = rgba[i * 4 + 3];
            out[i * 4 + 0] = static_cast<std::uint8_t>(b * a / 255);
            out[i * 4 + 1] = static_cast<std::uint8_t>(g * a / 255);
            out[i * 4 + 2] = static_cast<std::uint8_t>(r * a / 255);
            out[i * 4 + 3] = a;
        }

        POINT sourceTopLeft{ 0, 0 };
        SIZE size{ width, height };
        BLENDFUNCTION blend{ AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
        UpdateLayeredWindow(g_window, nullptr, nullptr, &size, g_memoryDC, &sourceTopLeft,
            0, &blend, ULW_ALPHA);
    }

    bool shouldClose() {
        return g_closeWanted;
    }

    void destroy() {
        if (g_bitmap)   DeleteObject(g_bitmap);
        if (g_memoryDC) DeleteDC(g_memoryDC);
        if (g_window)   DestroyWindow(g_window);
        UnregisterClassW(kClassName, GetModuleHandleW(nullptr));
        g_bitmap = nullptr;
        g_memoryDC = nullptr;
        g_window = nullptr;
    }

}