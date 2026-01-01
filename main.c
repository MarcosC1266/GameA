#include <windows.h>
#include <emmintrin.h>
#include <stdio.h>
#include <psapi.h>

#include "main.h"
#include "performance.h"
#include "rendering.h"
//
// Created by MJVA_ on 24/12/2025.
//

BOOL gGameRunning;
HWND gHwnd;
PERFDATA gPerformanceData;
PLAYER gPlayer;

int _stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nShowCmd) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nShowCmd);

    MSG msg;
    int64_t frequency;
    int64_t startTime;
    int64_t endTime;
    HMODULE ntDllModuel;
    HANDLE currentProcess = GetCurrentProcess();


    if ((ntDllModuel = GetModuleHandleA("ntdll.dll")) == NULL) {
        MessageBoxA(NULL, "Couldn't load ntdll.dll", "Error!", MB_ICONERROR | MB_OK);
        goto Exit;
    }

    if ((NtQueryTimerResolution = (_NtQueryTimerResolution) GetProcAddress(ntDllModuel, "NtQueryTimerResolution")) == NULL) {
        MessageBoxA(NULL, "Couldn't find NtQueryTimerResolution in ntdll.dll", "Error!", MB_ICONERROR | MB_OK);
        goto Exit;
    }

    NtQueryTimerResolution(&gPerformanceData.minTimerResolution, &gPerformanceData.maxTimerResolution, &gPerformanceData.currentTimerResolution);
    GetSystemInfo(&gPerformanceData.systemInfo);
    GetSystemTimeAsFileTime((FILETIME*)&gPerformanceData.cpuData.prevSystemTime);

    if (GameHealthCheck() == TRUE) {
        MessageBox(NULL, "Game is already running", "Error", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if (timeBeginPeriod(1) == TIMERR_NOCANDO) {
        MessageBox(NULL, "Failed to set global timer resolution", "Error", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    SetPriorityClass(currentProcess, HIGH_PRIORITY_CLASS);

    if (CreateMainWindow(hInstance) != ERROR_SUCCESS) {
        goto Exit;
    }

    ShowWindow(gHwnd, TRUE);
    QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
    gPerformanceData.debugMode = TRUE;

    if (CreateBackBuffer() != ERROR_SUCCESS) {
        MessageBox(NULL, "Failed to allocated memory for drawing surface!", "Error", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }


    gGameRunning = TRUE;
    gPlayer.posX = 25;
    gPlayer.posY = 25;

    while (gGameRunning) {

        QueryPerformanceCounter((LARGE_INTEGER*)&startTime);
        while (PeekMessage(&msg, gHwnd, 0, 0, PM_REMOVE)) {
            DispatchMessage(&msg);
        }
        PlayerInput();
        Render(gPlayer,gPerformanceData,gHwnd);
        QueryPerformanceCounter((LARGE_INTEGER*)&endTime);
        CalculatePerformance(&gPerformanceData, currentProcess, startTime, &endTime, frequency);
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
       WS_VISIBLE,
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

    const int windowStyleResult = SetWindowLong(gHwnd, GWL_STYLE,  WS_VISIBLE);

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
    const int16_t escapePressed = GetAsyncKeyState(VK_ESCAPE);
    const int16_t debugMode= GetAsyncKeyState(VK_F3);
    static int16_t hDebugMode;
    const int16_t leftPressed = GetAsyncKeyState(VK_LEFT);
    const int16_t rightPressed = GetAsyncKeyState(VK_RIGHT);
    const int16_t upPressed = GetAsyncKeyState(VK_UP);
    const int16_t downPressed = GetAsyncKeyState(VK_DOWN);

    if (leftPressed && gPlayer.posX > 0) {
        gPlayer.posX--;
    }
    if (rightPressed && gPlayer.posX < GAME_RES_WIDTH -16) {
        gPlayer.posX++;
    }

    if (upPressed && gPlayer.posY > 0) {
        gPlayer.posY--;
    }
    if (downPressed && gPlayer.posY < GAME_RES_HEIGHT - 16) {
        gPlayer.posY++;
    }

    if (escapePressed) {
        SendMessage(gHwnd, WM_CLOSE, 0, 0);
    }

    if (debugMode && !hDebugMode) {
        gPerformanceData.debugMode = !gPerformanceData.debugMode;
    }

    hDebugMode = debugMode;
}


