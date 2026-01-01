#include <windows.h>
#include "main.h"
#include <emmintrin.h>
#include <stdio.h>
#include <psapi.h>
//
// Created by MJVA_ on 24/12/2025.
//

BOOL gGameRunning;
HWND gHwnd;
GAMEBITMAP gBackBuffer;
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
    int64_t elapsedTime;
    int64_t elapsedTimeAcc = 0;
    int64_t targetTimeAcc = 0;
    HMODULE ntDllModuel;

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

    if (GameHealthCheck() == TRUE) {
        MessageBox(NULL, "Game is already running", "Error", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if (CreateMainWindow(hInstance) != ERROR_SUCCESS) {
        goto Exit;
    }

    ShowWindow(gHwnd, TRUE);
    QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
    gPerformanceData.debugMode = TRUE;

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

    gGameRunning = TRUE;
    gPlayer.posX = 25;
    gPlayer.posY = 25;

    while (gGameRunning) {

        QueryPerformanceCounter((LARGE_INTEGER*)&startTime);
        while (PeekMessage(&msg, gHwnd, 0, 0, PM_REMOVE)) {
            DispatchMessage(&msg);
        }
        PlayerInput();
        Render();

        QueryPerformanceCounter((LARGE_INTEGER*)&endTime);
        elapsedTime = endTime - startTime;
        elapsedTime *= 1000000;
        elapsedTime /= frequency;
        gPerformanceData.totalFramesRendered ++;
        elapsedTimeAcc += elapsedTime;

        while (elapsedTime < TARGET_MICROSECONDS_PER_FRAME) {
            elapsedTime = endTime - startTime;
            elapsedTime *= 1000000;
            elapsedTime /= frequency;
            QueryPerformanceCounter((LARGE_INTEGER*)&endTime);

            if (elapsedTime < (TARGET_MICROSECONDS_PER_FRAME - ((uint64_t)gPerformanceData.currentTimerResolution * 0.1f * 16.5f))) {
                Sleep(1);
            }
        }
        targetTimeAcc += elapsedTime;


        if (gPerformanceData.totalFramesRendered % CALCULATE_AVG_FPS_X_FRAMES == 0) {
            HANDLE currentProcess = GetCurrentProcess();

            GetSystemTimeAsFileTime((FILETIME*)&gPerformanceData.cpuData.systemTime);
            GetProcessTimes(
                currentProcess,
                &gPerformanceData.cpuData.creationTime, &gPerformanceData.cpuData.exitTime,
                (FILETIME*)&gPerformanceData.cpuData.kernelTime, (FILETIME*)&gPerformanceData.cpuData.userTime
                );
            GetProcessHandleCount(currentProcess, &gPerformanceData.handleCount);
            GetProcessMemoryInfo(currentProcess, (PROCESS_MEMORY_COUNTERS*)&gPerformanceData.memInfo, sizeof(gPerformanceData.memInfo));

            gPerformanceData.cpuData.cpuUsage = (gPerformanceData.cpuData.kernelTime - gPerformanceData.cpuData.prevKernelTime) + (gPerformanceData.cpuData.userTime - gPerformanceData.cpuData.prevUserTime);
            gPerformanceData.cpuData.cpuUsage /= (gPerformanceData.cpuData.systemTime - gPerformanceData.cpuData.prevSystemTime);
            gPerformanceData.cpuData.cpuUsage /= gPerformanceData.systemInfo.dwNumberOfProcessors;
            gPerformanceData.cpuData.cpuUsage *= 100;

            gPerformanceData.msFrame = (elapsedTimeAcc/CALCULATE_AVG_FPS_X_FRAMES) * 0.001f;
            gPerformanceData.maxFrame = 1.0f/((elapsedTimeAcc/CALCULATE_AVG_FPS_X_FRAMES) * 0.000001f);
            gPerformanceData.avgFrame = 1.0f/((targetTimeAcc/CALCULATE_AVG_FPS_X_FRAMES) * 0.000001f);
            gPerformanceData.cpuData.prevKernelTime = gPerformanceData.cpuData.kernelTime;
            gPerformanceData.cpuData.prevUserTime = gPerformanceData.cpuData.userTime;
            gPerformanceData.cpuData.prevSystemTime = gPerformanceData.cpuData.systemTime;

            elapsedTimeAcc  = 0;
            targetTimeAcc = 0;

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

void Render() {
    __m128i pixelLoad = _mm_setr_epi8(
        0xff, 0x99, 0x00, 0xff,
        0xff, 0x99, 0x00, 0xff,
        0xff, 0x99, 0x00, 0xff,
        0xff, 0x99, 0x00, 0xff
        );

    ClearScreen(pixelLoad);

    int32_t screenX =gPlayer.posX;
    int32_t screenY =gPlayer.posY;
    int32_t startingPixel = ((GAME_RES_WIDTH *  GAME_RES_HEIGHT) - GAME_RES_WIDTH) - (GAME_RES_WIDTH * screenY) + screenX;

    for (int32_t y = 0; y < 16; y++) {
        for (int32_t x = 0; x < 16; x++) {
            memset((PIXEL32*)gBackBuffer.memory + (uintptr_t)startingPixel + x - ((uintptr_t)GAME_RES_WIDTH * y), 0xff,sizeof(PIXEL32));
        }
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

    if (gPerformanceData.debugMode) {
        PrintDebugInfo(DeviceContext);
    }


    ReleaseDC(gHwnd, DeviceContext);
}

void ClearScreen(__m128i color) {
    __m128i* buffer = (__m128i*)gBackBuffer.memory;
    int totalPixels = GAME_RES_WIDTH * GAME_RES_HEIGHT;

    for (int x = 0; x < totalPixels / 4; x++) {
        _mm_store_si128(buffer + x, color);
    }
}

void PrintDebugInfo(HDC deviceContext) {
    SelectObject(deviceContext, (HFONT)GetStockObject(ANSI_FIXED_FONT));

    char debugTextBuff[64] = {0};

    sprintf_s(debugTextBuff, _countof(debugTextBuff), "FPS Max:                 %.01f", gPerformanceData.maxFrame);
    TextOutA(deviceContext, 0,0, debugTextBuff, (int)strlen(debugTextBuff));

    sprintf_s(debugTextBuff, _countof(debugTextBuff), "FPS Avg:                 %.01f", gPerformanceData.avgFrame);
    TextOutA(deviceContext, 0,13, debugTextBuff, (int)strlen(debugTextBuff));

    sprintf_s(debugTextBuff, _countof(debugTextBuff), "Frame time:              %.01f ms", gPerformanceData.msFrame);
    TextOutA(deviceContext, 0,26, debugTextBuff, (int)strlen(debugTextBuff));

    sprintf_s(debugTextBuff, _countof(debugTextBuff), "Min. Timer Resolution:   %.01f ms", gPerformanceData.minTimerResolution / 10000.0f);
    TextOutA(deviceContext, 0,39, debugTextBuff, (int)strlen(debugTextBuff));

    sprintf_s(debugTextBuff, _countof(debugTextBuff), "Max. Timer Resolution:   %.01f ms", gPerformanceData.maxTimerResolution / 10000.0f);
    TextOutA(deviceContext, 0,52, debugTextBuff, (int)strlen(debugTextBuff));

    sprintf_s(debugTextBuff, _countof(debugTextBuff), "Cur. Timer Resolution:   %.01f ms", gPerformanceData.currentTimerResolution / 10000.0f);
    TextOutA(deviceContext, 0,65, debugTextBuff, (int)strlen(debugTextBuff));

    sprintf_s(debugTextBuff, _countof(debugTextBuff), "Monitor Resolution:      %dx%d", gPerformanceData.monitorWidth, gPerformanceData.monitorHeight);
    TextOutA(deviceContext, 0,78, debugTextBuff, (int)strlen(debugTextBuff));

    sprintf_s(debugTextBuff, _countof(debugTextBuff), "Handles:                 %u", gPerformanceData.handleCount);
    TextOutA(deviceContext, 0,91, debugTextBuff, (int)strlen(debugTextBuff));

    sprintf_s(debugTextBuff, _countof(debugTextBuff), "Memory:                  %lluKB", gPerformanceData.memInfo.PrivateUsage / 1024);
    TextOutA(deviceContext, 0,104, debugTextBuff, (int)strlen(debugTextBuff));

    sprintf_s(debugTextBuff, _countof(debugTextBuff), "CPU Usage:               %.01f%%", gPerformanceData.cpuData.cpuUsage);
    TextOutA(deviceContext, 0,117, debugTextBuff, (int)strlen(debugTextBuff));
}


