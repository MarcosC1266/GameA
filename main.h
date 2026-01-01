//
// Created by MJVA_ on 27/12/2025.
//

#ifndef GAMEA_MAIN_H
#define GAMEA_MAIN_H
#include <windows.h>
#include <stdint.h>
#include <emmintrin.h>

#define GAME_NAME "Game A"


typedef LONG(NTAPI* _NtQueryTimerResolution) (OUT PULONG MinimunResolution, OUT PULONG MaximunResolution, OUT PULONG CurrentResolution);
_NtQueryTimerResolution NtQueryTimerResolution;

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
DWORD CreateMainWindow(HINSTANCE Instance);
BOOL GameHealthCheck(void);
void PlayerInput(void);
#endif //GAMEA_MAIN_H
