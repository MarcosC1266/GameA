//
// Created by MJVA_ on 27/12/2025.
//

#ifndef GAMEA_MAIN_H
#define GAMEA_MAIN_H
#include <windows.h>
#include <stdint.h>
#include <emmintrin.h>

#define GAME_NAME "Game A"
#define GAME_RES_WIDTH 384
#define GAME_RES_HEIGHT 240
#define GAME_BPP 32
#define GAME_DRAWING_AREA_MS (GAME_RES_WIDTH * GAME_RES_HEIGHT * (GAME_BPP / 8))
#define CALCULATE_AVG_FPS_X_FRAMES 100
#define TARGET_MICROSECONDS_PER_FRAME 8333

typedef LONG(NTAPI* _NtQueryTimerResolution) (OUT PULONG MinimunResolution, OUT PULONG MaximunResolution, OUT PULONG CurrentResolution);
_NtQueryTimerResolution NtQueryTimerResolution;

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
    float avgFrame;
    float maxFrame;
    uint64_t frequency;
    uint64_t elapsedTime;
    BOOL debugMode;
    float msFrame;
    MONITORINFO monitorInfo;
    int32_t monitorWidth ;
    int32_t monitorHeight;
    LONG minTimerResolution;
    LONG maxTimerResolution;
    LONG currentTimerResolution;
} PERFDATA;

typedef struct PLAYER {
    int32_t playerId;
    char name[12];
    int32_t posX;
    int32_t posY;
    int32_t hp;
    int32_t maxHp;
    int32_t strength;
    int32_t defense;
}PLAYER;

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
DWORD CreateMainWindow(HINSTANCE Instance);
BOOL GameHealthCheck(void);
void PlayerInput(void);
void Render(void);
void PrintDebugInfo(HDC deviceContext);
void ClearScreen(__m128i color);
#endif //GAMEA_MAIN_H
