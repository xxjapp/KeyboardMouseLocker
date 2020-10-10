#pragma once

int __cdecl debugPrintf(const char* format, ...);
bool switchFullscreen(HWND hwnd, WINDOWPLACEMENT& wpPrev);
