#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include "./listing_70.cpp"

int main(int argCount, char **Args)
{

    int64_t millisecondsToWait = 1000;

    if(argCount == 2){
        millisecondsToWait = atol(Args[1]);
    }

    int64_t OSFreq = GetOSTimerFreq();

    printf("OS Freq %llu\n", OSFreq);

    uint64_t CPUStart = ReadCPUTmer();
    uint64_t OSStart = ReadOSTimer();
    uint64_t OSEnd = 0;
    uint64_t OSElapsed = 0;

    uint64_t OSWaitTime = OSFreq * millisecondsToWait / 1000;

    while (OSElapsed < OSWaitTime)
    {
        OSEnd = ReadOSTimer();
        OSElapsed = OSEnd - OSStart;
    }

    uint64_t CPUEnd = ReadCPUTmer();
    uint64_t CPUElapsed = CPUEnd - CPUStart;
    uint64_t CPUFreq = 0;

    if(OSElapsed){
        CPUFreq = OSFreq * CPUElapsed / OSElapsed;
    }

    printf("OS Timer: %llu -> %llu = %llu elapsed\n", OSStart, OSEnd, OSElapsed);
    printf("OS Seconds: %.4f\n", (double)OSElapsed / (double)OSFreq);
    printf("CPU Timer: %llu -> %llu = %llu elapsed\n", CPUStart, CPUEnd, CPUElapsed);
    printf("CPU Freq: %llu\n", CPUFreq);

    return 0;
}
