#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include "./listing_70.cpp"

int main(void)
{
    int64_t OSFreq = GetOSTimerFreq();

    printf("OS Freq %llu\n", OSFreq);

    uint64_t OSStart = ReadOSTimer();
    uint64_t OSEnd = 0;
    uint64_t OSElapsed = 0;

    while (OSElapsed < OSFreq)
    {
        OSEnd = ReadOSTimer();
        OSElapsed = OSEnd - OSStart;
    }

    printf("OS Timer: %llu -> %llu = %llu elapsed\n", OSStart, OSEnd, OSElapsed);
    printf("OS Seconds: %.4f\n", (double)OSElapsed / (double)OSFreq);

    return 0;
}
