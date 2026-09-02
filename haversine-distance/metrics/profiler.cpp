#include "./profiler.hpp"

uint64_t GetOSTimerFreq(void)
{
    return 1000000;
}

uint64_t ReadOSTimer(void)
{
    struct timeval Value;

    gettimeofday(&Value, 0);

    uint64_t result = GetOSTimerFreq() * (uint64_t)Value.tv_sec + (uint64_t)Value.tv_usec;

    return result;
}

inline uint64_t ReadCPUTmer(void)
{
    return __rdtsc();
}

uint64_t EstimateCPUFrequency(uint64_t msToWait)
{
    uint64_t OSFreq = GetOSTimerFreq();
    uint64_t OSElapsed = 0;
    uint64_t OSStart = 0;
    uint64_t OSEnd = 0;
    uint64_t CPUElapsed = 0;
    uint64_t CPUStart = 0;
    uint64_t CPUEnd = 0;

    CPUStart = ReadCPUTmer();
    OSStart = ReadOSTimer();

    while (OSElapsed < msToWait)
    {
        OSEnd = ReadOSTimer();
        OSElapsed = OSEnd - OSStart;
    }

    CPUEnd = ReadCPUTmer();
    CPUElapsed = CPUEnd - CPUStart;

    uint64_t CPUFreq = 0;

    if (OSElapsed)
    {
        CPUElapsed = OSFreq * CPUElapsed / OSElapsed;
    }

    return CPUFreq;
}
