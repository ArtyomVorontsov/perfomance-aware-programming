#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "./buffer.h"

#define CONSTANT_STRING(String) {sizeof(String) - 1, (uint8_t *)(String)}

int32_t IsInBounds(buffer Source, uint64_t At)
{
    int32_t Result = (At < Source.Count);
    return Result;
}

int32_t AreEqual(buffer A, buffer B)
{
    if (A.Count != B.Count)
    {
        return false;
    }

    for (uint64_t Index = 0; Index < A.Count; ++Index)
    {
        if (A.Data[Index] != B.Data[Index])
        {
            return false;
        }
    }

    return true;
}

buffer AllocateBuffer(size_t Count)
{
    buffer Result = {};
    Result.Data = (uint8_t *)malloc(Count);
    if (Result.Data)
    {
        Result.Count = Count;
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to allocate %llu bytes.\n", Count);
    }

    return Result;
}

void FreeBuffer(buffer *Buffer)
{
    if (Buffer->Data)
    {
        free(Buffer->Data);
    }
    *Buffer = {};
}