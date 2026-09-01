#include <x86intrin.h>
#include <sys/time.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>


static uint64_t GetOSTimerFreq(void){
    return 1000000;
}

static uint64_t ReadOSTimer(void){
    struct timeval Value;
    
    gettimeofday(&Value, 0);

    uint64_t result = GetOSTimerFreq() * (uint64_t)Value.tv_sec + (uint64_t)Value.tv_usec;

    return result;
}

inline uint64_t ReadCPUTmer(void){
    return __rdtsc();
}

