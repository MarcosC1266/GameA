//
// Created by MJVA_ on 1/1/2026.
//
#include <windows.h>
#include <emmintrin.h>
#include <stdio.h>
#include <stdint.h>

#include "rendering.h"
#include "performance.h"

GAMEBITMAP gBackBuffer;

DWORD CreateBackBuffer() {
    DWORD result = ERROR_SUCCESS;

    gBackBuffer.bitMapInfo.bmiHeader.biSize = sizeof(gBackBuffer.bitMapInfo.bmiHeader);
    gBackBuffer.bitMapInfo.bmiHeader.biWidth= GAME_RES_WIDTH;
    gBackBuffer.bitMapInfo.bmiHeader.biHeight = GAME_RES_HEIGHT;
    gBackBuffer.bitMapInfo.bmiHeader.biBitCount = GAME_BPP;
    gBackBuffer.bitMapInfo.bmiHeader.biCompression = BI_RGB;
    gBackBuffer.bitMapInfo.bmiHeader.biPlanes = 1;

    gBackBuffer.memory = VirtualAlloc(NULL, GAME_DRAWING_AREA_MS, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (gBackBuffer.memory == NULL) {
        result = GetLastError();
        goto Exit;
    }

    memset(gBackBuffer.memory, 0x7f, GAME_DRAWING_AREA_MS);
    Exit:
        return result;
}


void Render(PLAYER player, PERFDATA perfData, HWND hWnd) {
    __m128i pixelLoad = _mm_setr_epi8(
        0xff, 0x99, 0x00, 0xff,
        0xff, 0x99, 0x00, 0xff,
        0xff, 0x99, 0x00, 0xff,
        0xff, 0x99, 0x00, 0xff
        );

    ClearScreen(pixelLoad);

    int32_t screenX = player.posX;
    int32_t screenY = player.posY;
    int32_t startingPixel = ((GAME_RES_WIDTH *  GAME_RES_HEIGHT) - GAME_RES_WIDTH) - (GAME_RES_WIDTH * screenY) + screenX;

    for (int32_t y = 0; y < 16; y++) {
        for (int32_t x = 0; x < 16; x++) {
            memset((PIXEL32*)gBackBuffer.memory + (uintptr_t)startingPixel + x - ((uintptr_t)GAME_RES_WIDTH * y), 0xff,sizeof(PIXEL32));
        }
    }

    HDC DeviceContext = GetDC(hWnd);

    StretchDIBits(
        DeviceContext,
        0, 0,
        perfData.monitorWidth,perfData.monitorHeight,
        0,0,
        GAME_RES_WIDTH,GAME_RES_HEIGHT,
        gBackBuffer.memory, &gBackBuffer.bitMapInfo,
        DIB_RGB_COLORS, SRCCOPY
        );

    if (perfData.debugMode) {
        PrintDebugInfo(DeviceContext, perfData);
    }

    ReleaseDC(hWnd, DeviceContext);
}

void ClearScreen(__m128i color) {
    __m128i* buffer = (__m128i*)gBackBuffer.memory;
    int totalPixels = GAME_RES_WIDTH * GAME_RES_HEIGHT;

    for (int x = 0; x < totalPixels / 4; x++) {
        _mm_store_si128(buffer + x, color);
    }
}

void PrintDebugInfo(HDC deviceContext, PERFDATA perfData) {
    SelectObject(deviceContext, (HFONT)GetStockObject(ANSI_FIXED_FONT));

    char debugTextBuff[64] = {0};

    sprintf_s(debugTextBuff, _countof(debugTextBuff), "FPS Max:                 %.01f", perfData.maxFrame);
    TextOutA(deviceContext, 0,0, debugTextBuff, (int)strlen(debugTextBuff));

    sprintf_s(debugTextBuff, _countof(debugTextBuff), "FPS Avg:                 %.01f", perfData.avgFrame);
    TextOutA(deviceContext, 0,13, debugTextBuff, (int)strlen(debugTextBuff));

    sprintf_s(debugTextBuff, _countof(debugTextBuff), "Frame time:              %.01f ms", perfData.msFrame);
    TextOutA(deviceContext, 0,26, debugTextBuff, (int)strlen(debugTextBuff));

    sprintf_s(debugTextBuff, _countof(debugTextBuff), "Min. Timer Resolution:   %.01f ms", perfData.minTimerResolution / 10000.0f);
    TextOutA(deviceContext, 0,39, debugTextBuff, (int)strlen(debugTextBuff));

    sprintf_s(debugTextBuff, _countof(debugTextBuff), "Max. Timer Resolution:   %.01f ms", perfData.maxTimerResolution / 10000.0f);
    TextOutA(deviceContext, 0,52, debugTextBuff, (int)strlen(debugTextBuff));

    sprintf_s(debugTextBuff, _countof(debugTextBuff), "Cur. Timer Resolution:   %.01f ms", perfData.currentTimerResolution / 10000.0f);
    TextOutA(deviceContext, 0,65, debugTextBuff, (int)strlen(debugTextBuff));

    sprintf_s(debugTextBuff, _countof(debugTextBuff), "Monitor Resolution:      %dx%d", perfData.monitorWidth, perfData.monitorHeight);
    TextOutA(deviceContext, 0,78, debugTextBuff, (int)strlen(debugTextBuff));

    sprintf_s(debugTextBuff, _countof(debugTextBuff), "Handles:                 %u", perfData.handleCount);
    TextOutA(deviceContext, 0,91, debugTextBuff, (int)strlen(debugTextBuff));

    sprintf_s(debugTextBuff, _countof(debugTextBuff), "Memory:                  %lluKB", perfData.memInfo.PrivateUsage / 1024);
    TextOutA(deviceContext, 0,104, debugTextBuff, (int)strlen(debugTextBuff));

    sprintf_s(debugTextBuff, _countof(debugTextBuff), "CPU Usage:               %.01f%%", perfData.cpuData.cpuUsage);
    TextOutA(deviceContext, 0,117, debugTextBuff, (int)strlen(debugTextBuff));
}