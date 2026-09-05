#ifndef __PROFILER_HPP

#define __PROFILER_HPP 1

#include <x86intrin.h>
#include <sys/time.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <malloc.h>

uint64_t GetOSTimerFreq(void);
uint64_t ReadOSTimer(void);
uint64_t EstimateCPUFrequency(uint64_t msToWait);
inline uint64_t ReadCPUTmer(void);

#define PROFILE_FUNCTION() \
    FunctionGuard guard(__FUNCTION__)

#define NameConcat2(A, B) A##B
#define NameConcat(A, B) NameConcat2(A, B)
#define TimeBlock(Name) FunctionGuard NameConcat(Block, __LINE__)(Name, __COUNTER__)
#define TimeFunction TimeBlock(__func__)

struct FunctionGuard
{
    const char *name;
    uint64_t index;

    FunctionGuard(const char *name, uint32_t index);
    ~FunctionGuard();
};

struct ProfilerRecord
{
    const char *name;
    uint64_t start;
    uint64_t end;
    uint64_t elapsed;
};

struct ProfilerData
{
    ProfilerRecord **records;
    uint64_t recordsAmount;
    uint64_t totalElapsed;
    uint64_t start;
    uint64_t end;
    uint64_t allocatedRecordsBufferSize;
};

void printProfilerData();
void beginProfile();
void endAndPrintProfile();

#endif