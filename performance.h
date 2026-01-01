//
// Created by MJVA_ on 1/1/2026.
//

#ifndef GAMEA_PERFORMACE_H
#define GAMEA_PERFORMACE_H


#include <windows.h>
#include <stdint.h>
#include <psapi.h>

#define GAME_RES_WIDTH 384
#define GAME_RES_HEIGHT 240
#define GAME_BPP 32
#define GAME_DRAWING_AREA_MS (GAME_RES_WIDTH * GAME_RES_HEIGHT * (GAME_BPP / 8))
#define CALCULATE_AVG_FPS_X_FRAMES 100
#define TARGET_MICROSECONDS_PER_FRAME 16667ULL

typedef struct CPU_DATA{
    double cpuUsage;
    FILETIME creationTime;
    FILETIME exitTime;
    uint64_t kernelTime;
    uint64_t userTime;
    uint64_t systemTime;
    uint64_t prevUserTime;
    uint64_t prevKernelTime;
    uint64_t prevSystemTime;
}CPU_DATA;

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
    ULONG minTimerResolution;
    ULONG maxTimerResolution;
    ULONG currentTimerResolution;
    DWORD handleCount;
    PROCESS_MEMORY_COUNTERS_EX memInfo;
    SYSTEM_INFO systemInfo;
    CPU_DATA cpuData;
} PERFDATA;

void CalculatePerformance(PERFDATA* ptrPerData,HANDLE currentProcess, int64_t startTime, int64_t* ptrEndTime, int64_t frequency);

#endif //GAMEA_PERFORMACE_H