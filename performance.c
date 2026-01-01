//
// Created by MJVA_ on 1/1/2026.
//

#include "performance.h"

#include <stdio.h>
#include <windows.h>

void CalculatePerformance(PERFDATA* ptrPerData,HANDLE currentProcess, int64_t startTime, int64_t* ptrEndTime, int64_t frequency) {
    
    PERFDATA perfData = *ptrPerData;
    uint64_t endTime = *ptrEndTime;
    
    int64_t elapsedTime;
    int64_t elapsedTimeAcc = 0;
    int64_t targetTimeAcc = 0;

    elapsedTime = endTime - startTime;
    elapsedTime *= 1000000;
    elapsedTime /= frequency;
    perfData.totalFramesRendered ++;
    elapsedTimeAcc += elapsedTime;

    while (elapsedTime < TARGET_MICROSECONDS_PER_FRAME) {
        elapsedTime = endTime - startTime;
        elapsedTime *= 1000000;
        elapsedTime /= frequency;
        QueryPerformanceCounter((LARGE_INTEGER*)&endTime);

        if (elapsedTime < (TARGET_MICROSECONDS_PER_FRAME * 0.75f)) {
            Sleep(1);
        }
    }
    targetTimeAcc += elapsedTime;
    *ptrEndTime = endTime;


    if (perfData.totalFramesRendered % CALCULATE_AVG_FPS_X_FRAMES == 0) {

        GetSystemTimeAsFileTime((FILETIME*)&perfData.cpuData.systemTime);
        GetProcessTimes(
        currentProcess,
        &perfData.cpuData.creationTime, &perfData.cpuData.exitTime,
        (FILETIME*)&perfData.cpuData.kernelTime, (FILETIME*)&perfData.cpuData.userTime
        );
        GetProcessHandleCount(currentProcess, &perfData.handleCount);
        GetProcessMemoryInfo(currentProcess, (PROCESS_MEMORY_COUNTERS*)&perfData.memInfo, sizeof(perfData.memInfo));

        perfData.cpuData.cpuUsage = (perfData.cpuData.kernelTime - perfData.cpuData.prevKernelTime) + (perfData.cpuData.userTime - perfData.cpuData.prevUserTime);
        perfData.cpuData.cpuUsage /= (perfData.cpuData.systemTime - perfData.cpuData.prevSystemTime);
        perfData.cpuData.cpuUsage /= perfData.systemInfo.dwNumberOfProcessors;
        perfData.cpuData.cpuUsage *= 100;

        perfData.msFrame = (elapsedTimeAcc) * 0.001f;
        perfData.maxFrame = 1.0f/((elapsedTimeAcc) * 0.000001f);
        perfData.avgFrame = 1.0f/((targetTimeAcc) * 0.000001f);
        perfData.cpuData.prevKernelTime = perfData.cpuData.kernelTime;
        perfData.cpuData.prevUserTime = perfData.cpuData.userTime;
        perfData.cpuData.prevSystemTime = perfData.cpuData.systemTime;
        
        elapsedTimeAcc = 0;
        targetTimeAcc = 0;
    }

    *ptrPerData = perfData;

}
