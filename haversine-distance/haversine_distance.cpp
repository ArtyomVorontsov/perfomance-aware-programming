#include "./haversine_distance.h"

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

    uint64_t StartupStart;
    uint64_t StartupEnd;
    uint64_t StartupElapsed;

    uint64_t ReadStart;
    uint64_t ReadEnd;
    uint64_t ReadElapsed;

    uint64_t MiscStart;
    uint64_t MiscEnd;
    uint64_t MiscElapsed;

    uint64_t ParseStart;
    uint64_t ParseEnd;
    uint64_t ParseElapsed;

    uint64_t SumStart;
    uint64_t SumEnd;
    uint64_t SumElapsed;

    uint64_t OutputStart;
    uint64_t OutputEnd;
    uint64_t OutputElapsed;

    uint64_t StartTime;
    uint64_t EndTime;
    uint64_t ElapsedTime;

    StartTime = ReadOSTimer();

    StartupStart = ReadCPUTmer();

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

            StartupEnd = ReadCPUTmer();
            StartupElapsed = StartupEnd - StartupStart;

            if (ParsedValues.Count)
            {
                haversine_pair *Pairs = (haversine_pair *)ParsedValues.Data;

                ParseStart = ReadCPUTmer();

                uint64_t PairCount = ParseHaversinePairs(file, MaxPairCount, Pairs);
                ParseEnd = ReadCPUTmer();
                ParseElapsed = ParseEnd - ParseStart;

                SumStart = ReadCPUTmer();

                double Sum = SumHaversineDistances(PairCount, Pairs);

                SumEnd = ReadCPUTmer();
                SumElapsed = SumEnd - SumStart;

                OutputStart = ReadCPUTmer();

                fprintf(stdout, "Input size: %llu\n", InputJSON.Count);
                fprintf(stdout, "Pair count: %llu\n", PairCount);
                fprintf(stdout, "Haversine sum: %.16f\n", Sum);

                OutputEnd = ReadCPUTmer();
                OutputElapsed = OutputEnd - OutputStart;

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

    EndTime = ReadOSTimer();

    ElapsedTime = EndTime - StartTime;

    uint64_t totalElapsed =
        StartupElapsed +
        ReadElapsed +
        MiscElapsed +
        ParseElapsed +
        SumElapsed +
        OutputElapsed;

    double percent = ((double)totalElapsed / 100);

    double StartupPercent = (double)StartupElapsed / percent;
    double ReadPercent = (double)ReadElapsed / percent;
    double MiscPercent = (double)MiscElapsed / percent;
    double ParsePercent = (double)ParseElapsed / percent;
    double SumPercent = (double)SumElapsed / percent;
    double OutputPercent = (double)OutputElapsed / percent;

    uint64_t cpuFreq = EstimateCPUFrequency(100);

    printf("\n");
    printf("Profiling info:\n");
    printf("Total time: %llums (CPU freq %llu)\n", ElapsedTime, cpuFreq);
    printf("Startup: %llu (%.4f%)\n", StartupElapsed, StartupPercent);
    printf("Read: %llu (%.4f%)\n", ReadElapsed, ReadPercent);
    printf("Misc: %llu (%.4f%)\n", MiscElapsed, MiscPercent);
    printf("Parse: %llu (%.4f%)\n", ParseElapsed, ParsePercent);
    printf("Sum: %llu (%.4f%)\n", SumElapsed, SumPercent);
    printf("Output: %llu (%.4f%)\n", OutputElapsed, OutputPercent);
    printf("\n");
    printf("Total: %llu (%.4f%)\n", totalElapsed, StartupPercent + ReadPercent + MiscPercent + ParsePercent + SumPercent + OutputPercent);

    return Result;
}