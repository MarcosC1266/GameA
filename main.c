#include <windows.h>
#include "main.h"
//
// Created by MJVA_ on 24/12/2025.
//

BOOL gGameRunning;
HWND gHwnd;
GAMEBITMAP gDrawingSurface;

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

    gDrawingSurface.bitMapInfo.bmiHeader.biSize = sizeof(gDrawingSurface.bitMapInfo.bmiHeader);
    gDrawingSurface.bitMapInfo.bmiHeader.biWidth= GAME_RES_WIDTH;
    gDrawingSurface.bitMapInfo.bmiHeader.biHeight = GAME_RES_HEIGHT;
    gDrawingSurface.bitMapInfo.bmiHeader.biBitCount = GAME_BPP;
    gDrawingSurface.bitMapInfo.bmiHeader.biCompression = BI_RGB;
    gDrawingSurface.bitMapInfo.bmiHeader.biPlanes = 1;

    gDrawingSurface.memory = VirtualAlloc(NULL, GAME_DRAWING_AREA_MS, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (gDrawingSurface.memory == NULL) {
        MessageBox(NULL, "Failed to allocated memory for drawing surface!", "Error", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }


    MSG msg;
    gGameRunning = TRUE;

    while (gGameRunning) {
        while (PeekMessage(&msg, gHwnd, 0, 0, PM_REMOVE)) {
            DispatchMessage(&msg);
        }

        PlayerInput();
        //Render();

        Sleep(1);
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
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = GAME_NAME "_WINDOWCLASS";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassExA(&wc)) {
        result = GetLastError();
        MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    gHwnd = CreateWindowEx(
      WS_EX_CLIENTEDGE,
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
    ShowWindow(gHwnd, TRUE);

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

void Render(){}

