#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <sys/stat.h>
#include "./haversine_distance.h"
#include "./haversine-formula/haversine-formula.h"
#include "./json_parser/parser/parser.h"
#include "./json_parser/tokenizer/tokenizer.h"
#include "./buffer/buffer.h"

struct haversine_pair
{
    double X0, Y0;
    double X1, Y1;
};

static buffer ReadEntireFile(char *FileName)
{
    buffer Result = {};

    FILE *File = fopen(FileName, "rb");
    if (File)
    {

        struct stat Stat;
        stat(FileName, &Stat);

        Result = AllocateBuffer(Stat.st_size);
        if (Result.Data)
        {
            if (fread(Result.Data, Result.Count, 1, File) != 1)
            {
                fprintf(stderr, "ERROR: Unable to read \"%s\".\n", FileName);
                FreeBuffer(&Result);
            }
        }

        fclose(File);
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to open \"%s\".\n", FileName);
    }

    return Result;
}

static double SumHaversineDistances(uint64_t PairCount, haversine_pair *Pairs)
{
    double Sum = 0;

    double SumCoef = 1 / (double)PairCount;
    for (uint64_t PairIndex = 0; PairIndex < PairCount; ++PairIndex)
    {
        haversine_pair Pair = Pairs[PairIndex];
        double EarthRadius = 6372.8;
        double Dist = ReferenceHaversine(Pair.X0, Pair.Y0, Pair.X1, Pair.Y1, EarthRadius);
        Sum += SumCoef * Dist;
    }

    return Sum;
}

static uint64_t ParseHaversinePairs(FILE *InputJSON, uint64_t MaxPairCount, haversine_pair *Pairs)
{
    uint64_t PairCount = 0;

    Lexer *lexerResult = lexer(InputJSON);
    JsonValue *JSON = parser(lexerResult);
    JsonArray *PairsArray = JSON->as.object->entries[0]->value->as.array;
    if (PairsArray)
    {

        for (size_t i = 0; i < PairsArray->count; i++)
        {
            JsonValue *Element = PairsArray->items[i];
            haversine_pair *Pair = Pairs + PairCount++;

            Pair->X0 = Element->as.object->entries[0]->value->as.number;
            Pair->Y0 = Element->as.object->entries[1]->value->as.number;
            Pair->X1 = Element->as.object->entries[2]->value->as.number;
            Pair->Y1 = Element->as.object->entries[3]->value->as.number;
        }
    }

    // FreeJSON(JSON);

    return PairCount;
}

int main(int ArgCount, char **Args)
{
    int Result = 1;

    if ((ArgCount == 2) || (ArgCount == 3))
    {
        buffer InputJSON = ReadEntireFile(Args[1]);
        FILE *file = fopen(Args[1], "rw");

        uint32_t MinimumJSONPairEncoding = 6 * 4;
        uint64_t MaxPairCount = InputJSON.Count / MinimumJSONPairEncoding;
        if (MaxPairCount)
        {
            buffer ParsedValues = AllocateBuffer(MaxPairCount * sizeof(haversine_pair));
            if (ParsedValues.Count)
            {
                haversine_pair *Pairs = (haversine_pair *)ParsedValues.Data;
                uint64_t PairCount = ParseHaversinePairs(file, MaxPairCount, Pairs);
                double Sum = SumHaversineDistances(PairCount, Pairs);

                fprintf(stdout, "Input size: %llu\n", InputJSON.Count);
                fprintf(stdout, "Pair count: %llu\n", PairCount);
                fprintf(stdout, "Haversine sum: %.16f\n", Sum);

                if (ArgCount == 3)
                {
                    buffer AnswersF64 = ReadEntireFile(Args[2]);
                    if (AnswersF64.Count >= sizeof(double))
                    {
                        double *AnswerValues = (double *)AnswersF64.Data;

                        fprintf(stdout, "\nValidation:\n");

                        uint64_t RefAnswerCount = (AnswersF64.Count - sizeof(double)) / sizeof(double);
                        if (PairCount != RefAnswerCount)
                        {
                            fprintf(stdout, "FAILED - pair count doesn't match %llu.\n", RefAnswerCount);
                        }

                        double RefSum = AnswerValues[RefAnswerCount];
                        fprintf(stdout, "Reference sum: %.16f\n", RefSum);
                        fprintf(stdout, "Difference: %.16f\n", Sum - RefSum);

                        fprintf(stdout, "\n");
                    }
                }
            }

            FreeBuffer(&ParsedValues);
        }
        else
        {
            fprintf(stderr, "ERROR: Malformed input JSON\n");
        }

        FreeBuffer(&InputJSON);

        Result = 0;
    }
    else
    {
        fprintf(stderr, "Usage: %s [haversine_input.json]\n", Args[0]);
        fprintf(stderr, "       %s [haversine_input.json] [answers.double]\n", Args[0]);
    }

    return Result;
}