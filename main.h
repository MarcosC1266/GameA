//
// Created by MJVA_ on 27/12/2025.
//

#ifndef GAMEA_MAIN_H
#define GAMEA_MAIN_H
#include <windows.h>

#define GAME_NAME "Game A"
#define GAME_RES_WIDTH 1920
#define GAME_RES_HEIGHT 1080
#define GAME_BPP 32
#define GAME_DRAWING_AREA_MS (GAME_RES_WIDTH * GAME_RES_HEIGHT * (GAME_BPP / 8))

typedef struct GAMEBITMAP {
    BITMAPINFO bitMapInfo;
    void* memory;
} GAMEBITMAP;

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
DWORD CreateMainWindow(HINSTANCE Instance);
BOOL GameHealthCheck(void);
void PlayerInput(void);
void Render(void);
#endif //GAMEA_MAIN_H
