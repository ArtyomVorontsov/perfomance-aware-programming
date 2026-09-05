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

extern ProfilerData *PROFILER_DATA = (ProfilerData *)malloc(sizeof(ProfilerData));

void addNewProfilerRecord(uint32_t index)
{

    // Allocate records buffer dynamically
    if (PROFILER_DATA->records == NULL)
    {
        PROFILER_DATA->records = (ProfilerRecord **)calloc(64, sizeof(ProfilerRecord *));
        PROFILER_DATA->allocatedRecordsBufferSize = 64;
    }
    else if (PROFILER_DATA->recordsAmount > 64)
    {
        PROFILER_DATA->allocatedRecordsBufferSize *= 2;
        PROFILER_DATA->records = (ProfilerRecord **)realloc(PROFILER_DATA->records,
                                                            sizeof(ProfilerRecord *) * PROFILER_DATA->allocatedRecordsBufferSize);
    }

    if (PROFILER_DATA->records[index] == NULL)
    {
        ProfilerRecord *profilerRecord = (ProfilerRecord *)calloc(1, sizeof(ProfilerRecord));
        PROFILER_DATA->records[index] = profilerRecord;
    }
    PROFILER_DATA->recordsAmount = PROFILER_DATA->recordsAmount < index ? index : PROFILER_DATA->recordsAmount;
}

FunctionGuard::FunctionGuard(const char *name, uint32_t index) : name(name)
{
    FunctionGuard::index = index;
    addNewProfilerRecord(index);
    PROFILER_DATA->records[FunctionGuard::index]->name = name;
    PROFILER_DATA->records[FunctionGuard::index]->start = ReadCPUTmer();
}

FunctionGuard::~FunctionGuard()
{

    PROFILER_DATA->records[FunctionGuard::index]->end = ReadCPUTmer();
    PROFILER_DATA->records[FunctionGuard::index]->elapsed +=
        PROFILER_DATA->records[FunctionGuard::index]->end - PROFILER_DATA->records[FunctionGuard::index]->start;
}

void printProfilerData()
{
    uint64_t totalElapsed = PROFILER_DATA->totalElapsed;
    printf("totalElapsed: %llu\n", totalElapsed);

    double percent = ((double)totalElapsed / 100);
    double totalPercent = 0;

    printf("\n");
    printf("Profiling info:\n");

    for (size_t i = 0; i <= PROFILER_DATA->recordsAmount; i++)
    {
        double p = (double)PROFILER_DATA->records[i]->elapsed / percent;

        printf("%s: %llu (%.4f%)\n", PROFILER_DATA->records[i]->name, PROFILER_DATA->records[i]->elapsed, p);
        totalPercent += p;
    }

    uint64_t cpuFreq = EstimateCPUFrequency(100);

    printf("\n");
    printf("Total: %llu (%.4f%)\n", totalElapsed, totalPercent);
}

void beginProfile()
{
    PROFILER_DATA->start = ReadCPUTmer();
}

void endAndPrintProfile()
{
    PROFILER_DATA->end = ReadCPUTmer();
    PROFILER_DATA->totalElapsed = PROFILER_DATA->end - PROFILER_DATA->start;

    printProfilerData();
}