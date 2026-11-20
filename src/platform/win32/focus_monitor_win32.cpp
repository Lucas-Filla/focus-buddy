#include "../focus_monitor.h"
#include <windows.h>

namespace focus {

    std::string activeWindowTitle() {
        HWND window = GetForegroundWindow();
        if (!window) return {};

        int length = GetWindowTextLengthW(window);
        if (length == 0) return {};

        std::wstring wide(length + 1, L'\0');
        int copied = GetWindowTextW(window, wide.data(), length + 1);
        wide.resize(copied);

        int bytes = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), static_cast<int>(wide.size()),
            nullptr, 0, nullptr, nullptr);
        std::string utf8(bytes, '\0');
        WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), static_cast<int>(wide.size()),
            utf8.data(), bytes, nullptr, nullptr);
        return utf8;
    }

    double idleSeconds() {
        LASTINPUTINFO info{};
        info.cbSize = sizeof(info);
        if (!GetLastInputInfo(&info)) return 0.0;
        DWORD now = GetTickCount();
        return (now - info.dwTime) / 1000.0;
    }

}