// KeyboardMouseLocker.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "KeyboardMouseLocker.h"
#include "KeyTable.h"
#include "Common.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND hMyWnd;                                    // created window
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR    lpCmdLine,
                      _In_ int       nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_KEYBOARDMOUSELOCKER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // check if already running
    HANDLE hMutex = CreateMutex(NULL, TRUE, szTitle);

    if (ERROR_ALREADY_EXISTS == GetLastError()) {
        // Program already running somewhere
        return 1; // Exit program
    }

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow)) {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_KEYBOARDMOUSELOCKER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    if (hMutex != NULL) {
        ReleaseMutex(hMutex);   // Explicitly release mutex
        CloseHandle(hMutex);    // close handle before terminating
    }

    return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KEYBOARDMOUSELOCKER));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    hInst = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) {
        return FALSE;
    }

    hMyWnd = hWnd;

    // Make window almost transparent
    SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hWnd, 0, 1, LWA_ALPHA);

    // set full screen
    WINDOWPLACEMENT wpPrev;
    switchFullscreen(hWnd, wpPrev);

    // hide cursor
    ShowCursor(FALSE);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

// SEE: Virtual-Key Codes
// https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
LRESULT CALLBACK LLKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    // to enable?
    PKBDLLHOOKSTRUCT hs = (PKBDLLHOOKSTRUCT)lParam;
    bool enable = hs->vkCode == VK_F12 || hs->vkCode == VK_LCONTROL || hs->vkCode == VK_RCONTROL || wParam == WM_KEYUP;

    // output log
    char* nCodeStr = nCode == HC_ACTION ? "HC_ACTION" : "?";
    char* wParamStr = wParam == WM_KEYDOWN ? "KD " : (wParam == WM_SYSKEYDOWN ? "SKD" : (wParam == WM_KEYUP ? "KU " : (wParam == WM_SYSKEYUP ? "SKU" : "?")));

    debugPrintf("%s %s vk = %-12s, scanCode = %ld, flags = %3ld %s\n", nCodeStr, wParamStr, keyTable[hs->vkCode], hs->scanCode, hs->flags, enable ? "" : "[DISABLED]");

    // enable: call next hook
    // disable: processed
    return enable ? CallNextHookEx(NULL, nCode, wParam, lParam) : 1;
}

HHOOK hookKeys = NULL;

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        debugPrintf("SetWindowsHookEx\n");
        hookKeys = SetWindowsHookEx(WH_KEYBOARD_LL, LLKeyboardProc, ((LPCREATESTRUCT)lParam)->hInstance, 0);
        break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId) {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        UnhookWindowsHookEx(hookKeys);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
