//
// Created by MJVA_ on 27/12/2025.
//

#ifndef GAMEA_MAIN_H
#define GAMEA_MAIN_H
#include <windows.h>
#include <stdint.h>

#define GAME_NAME "Game A"
#define GAME_RES_WIDTH 640
#define GAME_RES_HEIGHT 360
#define GAME_BPP 32
#define GAME_DRAWING_AREA_MS (GAME_RES_WIDTH * GAME_RES_HEIGHT * (GAME_BPP / 8))
#define CALCULATE_AVG_FPS_X_FRAMES 100

typedef struct GAMEBITMAP {
    BITMAPINFO bitMapInfo;
    void* memory;
} GAMEBITMAP;

typedef struct PIXEL32 {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
} PIXEL32;

typedef struct PERFDATA {
    uint64_t totalFramesRendered;
    uint32_t avgFrame;
    uint32_t maxFrame;
    LARGE_INTEGER frequency;
    LARGE_INTEGER startTime;
    LARGE_INTEGER endTime;
    LARGE_INTEGER elapsedTime;
    MONITORINFO monitorInfo;
    int32_t monitorWidth ;
    int32_t monitorHeight;
} PERFDATA;

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
DWORD CreateMainWindow(HINSTANCE Instance);
BOOL GameHealthCheck(void);
void PlayerInput(void);
void Render(void);
#endif //GAMEA_MAIN_H
