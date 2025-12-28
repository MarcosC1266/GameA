#include <windows.h>
#include "main.h"

#include <stdio.h>
//
// Created by MJVA_ on 24/12/2025.
//

BOOL gGameRunning;
HWND gHwnd;
GAMEBITMAP gBackBuffer;
PERFDATA gPerformanceData;

int _stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nShowCmd) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nShowCmd);

    if (GameHealthCheck() == TRUE) {
        MessageBox(NULL, "Game is already running", "Error", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if (CreateMainWindow(hInstance) != ERROR_SUCCESS) {
        goto Exit;
    }

    ShowWindow(gHwnd, TRUE);
    QueryPerformanceFrequency(&gPerformanceData.frequency);

    gBackBuffer.bitMapInfo.bmiHeader.biSize = sizeof(gBackBuffer.bitMapInfo.bmiHeader);
    gBackBuffer.bitMapInfo.bmiHeader.biWidth= GAME_RES_WIDTH;
    gBackBuffer.bitMapInfo.bmiHeader.biHeight = GAME_RES_HEIGHT;
    gBackBuffer.bitMapInfo.bmiHeader.biBitCount = GAME_BPP;
    gBackBuffer.bitMapInfo.bmiHeader.biCompression = BI_RGB;
    gBackBuffer.bitMapInfo.bmiHeader.biPlanes = 1;

    gBackBuffer.memory = VirtualAlloc(NULL, GAME_DRAWING_AREA_MS, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (gBackBuffer.memory == NULL) {
        MessageBox(NULL, "Failed to allocated memory for drawing surface!", "Error", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    memset(gBackBuffer.memory, 0x7f, GAME_DRAWING_AREA_MS);


    MSG msg;
    gGameRunning = TRUE;

    while (gGameRunning) {
        QueryPerformanceCounter(&gPerformanceData.startTime);
        while (PeekMessage(&msg, gHwnd, 0, 0, PM_REMOVE)) {
            DispatchMessage(&msg);
        }
        PlayerInput();
        Render();

        QueryPerformanceCounter(&gPerformanceData.endTime);
        gPerformanceData.elapsedTime.QuadPart = gPerformanceData.endTime.QuadPart - gPerformanceData.startTime.QuadPart;
        gPerformanceData.elapsedTime.QuadPart *= 1000000;
        gPerformanceData.elapsedTime.QuadPart /= gPerformanceData.frequency.QuadPart;

        Sleep(1);

        gPerformanceData.totalFramesRendered ++;

        if (gPerformanceData.totalFramesRendered % CALCULATE_AVG_FPS_X_FRAMES == 0) {
            char str[64] = {0};
            _snprintf_s(str, _countof(str),_TRUNCATE, "Elapsed microseconds: %lli\n", gPerformanceData.elapsedTime.QuadPart);
            OutputDebugStringA(str);
        }
    }

    Exit:
    return 0;
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

    LRESULT result = 0;

    switch (message) {

        case WM_CLOSE:
            // Initialize the window
            gGameRunning = FALSE;
            PostQuitMessage(0);
            break;
        default:
            result = DefWindowProc(hWnd, message, wParam, lParam);
    }
    return result;
}

DWORD CreateMainWindow(HINSTANCE Instance) {
    DWORD result = ERROR_SUCCESS;
    WNDCLASSEXA wc;


    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = 0;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = Instance;
    wc.hIcon = LoadIconA(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(255, 0, 255));
    wc.lpszMenuName = NULL;
    wc.lpszClassName = GAME_NAME "_WINDOWCLASS";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);


    if (!RegisterClassExA(&wc)) {
        result = GetLastError();
        MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    gHwnd = CreateWindowEx(
      0,
      wc.lpszClassName,
      GAME_NAME,
      WS_OVERLAPPEDWINDOW | WS_VISIBLE,
      CW_USEDEFAULT, CW_USEDEFAULT, GAME_RES_WIDTH, GAME_RES_HEIGHT,
      NULL, NULL, Instance, NULL
    );

    if (gHwnd == NULL) {
        result = GetLastError();
        MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    gPerformanceData.monitorInfo.cbSize = sizeof(MONITORINFO);

    const int monitorInfoResult = GetMonitorInfoA(MonitorFromWindow(gHwnd, MONITOR_DEFAULTTOPRIMARY), &gPerformanceData.monitorInfo);
    if (monitorInfoResult == 0) {
        result =  ERROR_INVALID_MONITOR_HANDLE;
        goto Exit;
    }

    gPerformanceData.monitorWidth = gPerformanceData.monitorInfo.rcMonitor.right - gPerformanceData.monitorInfo.rcMonitor.left;
    gPerformanceData.monitorHeight = gPerformanceData.monitorInfo.rcMonitor.bottom - gPerformanceData.monitorInfo.rcMonitor.top;

    const int windowStyleResult = SetWindowLong(gHwnd, GWL_STYLE, (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_OVERLAPPEDWINDOW);

    if (windowStyleResult == 0) {
        result = GetLastError();
        goto Exit;
    }

    SetWindowPos(
        gHwnd,
        HWND_TOP,
        gPerformanceData.monitorInfo.rcMonitor.left, gPerformanceData.monitorInfo.rcMonitor.top,
        gPerformanceData.monitorWidth, gPerformanceData.monitorHeight,
        SWP_FRAMECHANGED | SWP_NOACTIVATE);

    Exit:
    return result;
}

BOOL GameHealthCheck() {
    HANDLE Mutex= NULL;
    Mutex = CreateMutex(NULL, FALSE, GAME_NAME "_GameMutex");

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return TRUE;
    }
    return FALSE;
}

void PlayerInput() {
    SHORT EscapePressed = GetAsyncKeyState(VK_ESCAPE);
    if (EscapePressed) {
        SendMessage(gHwnd, WM_CLOSE, 0, 0);
    }
}

void Render() {

    PIXEL32 pixel = {0};

    pixel.blue = 0xff;
    pixel.green = 0x99;
    pixel.red = 0;
    pixel.alpha = 0xff;

    for (int x = 0; x < GAME_DRAWING_AREA_MS / 4; x++) {
        memcpy_s((PIXEL32*)gBackBuffer.memory + x,sizeof(PIXEL32), &pixel, sizeof(PIXEL32));
    }

    HDC DeviceContext = GetDC(gHwnd);


    StretchDIBits(
        DeviceContext,
        0, 0,
        gPerformanceData.monitorWidth,gPerformanceData.monitorHeight,
        0,0,
        GAME_RES_WIDTH,GAME_RES_HEIGHT,
        gBackBuffer.memory, &gBackBuffer.bitMapInfo,
        DIB_RGB_COLORS, SRCCOPY
        );

    ReleaseDC(gHwnd, DeviceContext);
}

