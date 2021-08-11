#include <iostream>
#include <Windows.h>
#include "resource.h"
#include "..\PoolData\PoolData.h"

#pragma once

//
// Global Objects
//
WNDCLASSEX HostWindowClass;
MSG loop_message;
HINSTANCE hInstance = GetModuleHandle(NULL);
HWND cpphwin_hwnd;
HWND wpf_hwnd;
typedef void (*GetPoolInformation_Ptr)(void);
typedef void (*GetNextHeapInformation_Ptr)(void);
typedef void (*GetNextAllocation_Ptr)(void);
typedef HWND(*CreateUserInterfaceFunc)(GetPoolInformation_Ptr, GetNextHeapInformation_Ptr, GetNextAllocation_Ptr);
CreateUserInterfaceFunc CreateUserInterface;
typedef void(*DisplayUserInterfaceFunc)(void);
DisplayUserInterfaceFunc DisplayUserInterface;
typedef void(*DestroyUserInterfaceFunc)(void);
DestroyUserInterfaceFunc DestroyUserInterface;
HMODULE dotNetGUILibrary;
RECT hwin_rect;

//
// Global Configs
//
const wchar_t cpphwinCN[] = L"CppMAppHostWinClass";
bool isHWindowRunning = false;
#define FIXED_WINDOW false
#define HWIN_TITLE L"PoolViewer"

/* 
    Host Window Callback 
*/
LRESULT
CALLBACK
HostWindowProc (
    _In_ HWND hwnd,
    _In_ UINT msg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    switch (msg)
    {
    case WM_CLOSE:
        //
        // Destroy WPF Control before Destorying Host Window
        //
        DestroyUserInterface();
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        isHWindowRunning = false;
        break;
    case WM_SIZE:
        //
        // Resize WPF Control on Host Window Resizing
        //
        if (wpf_hwnd != nullptr) {
            GetClientRect(cpphwin_hwnd, &hwin_rect);
            MoveWindow(wpf_hwnd, 0, 0, hwin_rect.right - hwin_rect.left, hwin_rect.bottom - hwin_rect.top, TRUE);
        }
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int
WINAPI
wWinMain (
    _In_ HINSTANCE hInstance,
    _In_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
)
{
    //
    // Create Icon Object From Resources
    //
    HICON app_icon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_APPICON));

    //
    // Define Our Host Window Class
    //
    HostWindowClass.cbSize = sizeof(WNDCLASSEX);
    HostWindowClass.lpfnWndProc = HostWindowProc;
    HostWindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    HostWindowClass.cbClsExtra = 0;
    HostWindowClass.style = 0;
    HostWindowClass.cbWndExtra = 0;
    HostWindowClass.hInstance = hInstance;
    HostWindowClass.hIcon = app_icon;
    HostWindowClass.hIconSm = app_icon;
    HostWindowClass.lpszClassName = cpphwinCN;
    HostWindowClass.lpszMenuName = NULL;
    printf("Starting\n");
    //
    // Register Window
    //
    if (!RegisterClassEx(&HostWindowClass))
    {
        printf("Error  %d\n", GetLastError());
        return 0;
    }

    /// Creating Unmanaged Host Window
    cpphwin_hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,
                                  cpphwinCN,
                                  HWIN_TITLE,
                                  WS_THICKFRAME | WS_OVERLAPPEDWINDOW,
                                  CW_USEDEFAULT,
                                  CW_USEDEFAULT,
                                  1800,
                                  1000,
                                  NULL,
                                  NULL,
                                  hInstance,
                                  NULL);

    //
    // Check if Window is valid
    //
    if (cpphwin_hwnd == NULL)
    {
        printf("Error  %d\n", GetLastError());
        return 0;
    }

    //
    // Making Window Fixed Size
    //
    if (FIXED_WINDOW) 
    { 
        ::SetWindowLong(cpphwin_hwnd,
                        GWL_STYLE,
                        GetWindowLong(cpphwin_hwnd, GWL_STYLE) & ~WS_SIZEBOX);
    }

    //
    // Center Host Window
    //
    RECT window_r;
    RECT desktop_r;
    GetWindowRect(cpphwin_hwnd, &window_r);
    GetWindowRect(GetDesktopWindow(), &desktop_r);
    int xPos = (desktop_r.right - (window_r.right - window_r.left)) / 2;
    int yPos = (desktop_r.bottom - (window_r.bottom - window_r.top)) / 2;

    //
    // Set Window Position
    //
    ::SetWindowPos(cpphwin_hwnd, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

    //
    // Load .Net UI Library
    //
    dotNetGUILibrary = LoadLibrary(L"ManagedUIKitWPF.dll");
    CreateUserInterface = (CreateUserInterfaceFunc)GetProcAddress(dotNetGUILibrary, "CreateUserInterface");
    DisplayUserInterface = (DisplayUserInterfaceFunc)GetProcAddress(dotNetGUILibrary, "DisplayUserInterface");
    DestroyUserInterface = (DestroyUserInterfaceFunc)GetProcAddress(dotNetGUILibrary, "DestroyUserInterface");

    //
    // Create .Net GUI
    //
    wpf_hwnd = CreateUserInterface(
        (GetPoolInformation_Ptr)&GetPoolInformation,
        (GetNextHeapInformation_Ptr)&GetNextHeapInformation,
        (GetNextAllocation_Ptr)&GetNextAllocation);
    
    //
    // Set Thread to STA
    //
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    //
    // Check if WPF Window is valid
    //
    if (wpf_hwnd != nullptr) {
        //
        // Disable Host Window Updates & Draws
        //
        SendMessage(cpphwin_hwnd, WM_SETREDRAW, FALSE, 0);

        //
        // Disable Host Window Double Buffering
        //
        long dwExStyle = GetWindowLong(cpphwin_hwnd, GWL_EXSTYLE);
        dwExStyle &= ~WS_EX_COMPOSITED;
        SetWindowLong(cpphwin_hwnd, GWL_EXSTYLE, dwExStyle);

        //
        // Set WPF Window to a Child Control
        //
        SetWindowLong(wpf_hwnd, GWL_STYLE, WS_CHILD);

        //
        // Get host client area rect
        //
        GetClientRect(cpphwin_hwnd, &hwin_rect);

        //
        // Set WPF Control Order, Size and Position
        //
        MoveWindow(wpf_hwnd, 0, 0, hwin_rect.right - hwin_rect.left, hwin_rect.bottom - hwin_rect.top, TRUE);
        SetWindowPos(wpf_hwnd, HWND_TOP, 0, 0, hwin_rect.right - hwin_rect.left, hwin_rect.bottom - hwin_rect.top, SWP_NOMOVE);

        //
        // Set WPF as A Child to Host Window...
        //
        SetParent(wpf_hwnd, cpphwin_hwnd);

        //
        // Show window
        //
        ShowWindow(wpf_hwnd, SW_RESTORE);

        //
        // Display WPF Control by Reseting its Opacity
        //
        DisplayUserInterface();
    }

    //
    // Display Window
    //
    ShowWindow(cpphwin_hwnd, SW_SHOW);
    UpdateWindow(cpphwin_hwnd);
    BringWindowToTop(cpphwin_hwnd);
    isHWindowRunning = true;

    //
    // Add Message Loop
    //
    while (GetMessage(&loop_message, NULL, 0, 0) > 0 && isHWindowRunning)
    {
        TranslateMessage(&loop_message);
        DispatchMessage(&loop_message);
    }

    //
    // Clean Up
    //
    printf("C++ Main App Finished.\n");
    FreeLibrary(dotNetGUILibrary);
    Sleep(500);
    return 0;
}