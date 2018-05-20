#include "stdafx.h"
#include "Common.h"

#include <stdio.h>

#include <string>

const std::string currentDateTime() {
    SYSTEMTIME t;
    GetLocalTime(&t);

    char currentTime[30] = "";
    sprintf_s(currentTime, "%04d-%02d-%02d %02d:%02d:%02d.%03d", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);

    return std::string(currentTime);
}

void output2File(char* str, size_t size) {
    static FILE *fp = NULL;

    if (fp == NULL) {
        errno_t err = fopen_s(&fp, "/tmp/KeyboardMouseLocker.log", "w");

        if (err != 0) {
            strerror_s(str, size, err);
            MessageBoxA(NULL, str, NULL, MB_ICONERROR);
            exit(2);
        }
    }

    fputs(str, fp);
    fflush(fp);

}

int __cdecl debugPrintf(const char *format, ...) {
    char str[1024];
    snprintf(str, sizeof(str), (currentDateTime() + " ").c_str());

    size_t len0 = strlen(str);
    char *p = str + len0;

    va_list argptr;
    va_start(argptr, format);
    int ret = vsnprintf(p, sizeof(str) - len0, format, argptr);
    va_end(argptr);

    OutputDebugStringA(str);
    output2File(str, sizeof(str));

    return int(len0) + ret;
}

LONG width(const RECT *rect) {
    return rect->right - rect->left;
}

LONG height(const RECT *rect) {
    return rect->bottom - rect->top;
}

// SEE: https://blogs.msdn.microsoft.com/oldnewthing/20100412-00/?p=14353/

bool switchFullscreen(HWND hwnd, WINDOWPLACEMENT &wpPrev) {
    DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
    bool gotoFullscreen = (dwStyle & WS_OVERLAPPEDWINDOW) != 0;

    if (gotoFullscreen) {
        MONITORINFO mi = { sizeof(mi) };

        if (GetWindowPlacement(hwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi)) {
            SetWindowLong(hwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(hwnd, HWND_TOP,
                         mi.rcMonitor.left, mi.rcMonitor.top,
                         width(&mi.rcMonitor), height(&mi.rcMonitor),
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    } else {
        SetWindowLong(hwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(hwnd, &wpPrev);
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }

    return gotoFullscreen;
}
