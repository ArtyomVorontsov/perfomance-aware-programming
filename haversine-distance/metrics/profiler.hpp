#include <x86intrin.h>
#include <sys/time.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

uint64_t GetOSTimerFreq(void);
uint64_t ReadOSTimer(void);
uint64_t EstimateCPUFrequency(uint64_t msToWait);
inline uint64_t ReadCPUTmer(void);