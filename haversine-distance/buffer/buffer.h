#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

struct buffer
{
    size_t Count;
    uint8_t *Data;
};


int32_t IsInBounds(buffer Source, uint64_t At);
int32_t AreEqual(buffer A, buffer B);
buffer AllocateBuffer(size_t Count);
void FreeBuffer(buffer *Buffer);
