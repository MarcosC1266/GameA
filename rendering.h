//
// Created by MJVA_ on 1/1/2026.
//

#ifndef GAMEA_RENDERING_H
#define GAMEA_RENDERING_H

#include <emmintrin.h>
#include <windows.h>
#include "player.h"
#include "performance.h"

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

DWORD CreateBackBuffer();
void Render(PLAYER player, PERFDATA perfData, HWND hWnd);
void PrintDebugInfo(HDC deviceContext, PERFDATA perfData);
void ClearScreen(__m128i color);
#endif //GAMEA_RENDERING_H