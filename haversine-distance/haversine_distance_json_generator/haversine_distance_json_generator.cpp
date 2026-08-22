#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include "../haversine-formula/haversine-formula.h"

struct random_series
{
    uint64_t A, B, C, D;
};

static uint64_t RotateLeft(uint64_t V, int Shift)
{
    uint64_t Result = ((V << Shift) | (V >> (64 - Shift)));
    return Result;
}

static uint64_t RandomU64(random_series *Series)
{
    uint64_t A = Series->A;
    uint64_t B = Series->B;
    uint64_t C = Series->C;
    uint64_t D = Series->D;

    uint64_t E = A - RotateLeft(B, 27);

    A = (B ^ RotateLeft(C, 17));
    B = (C + D);
    C = (D + E);
    D = (E + A);

    Series->A = A;
    Series->B = B;
    Series->C = C;
    Series->D = D;

    return D;
}

static random_series Seed(uint64_t Value)
{
    random_series Series = {};

    // NOTE(casey): This is the seed pattern for JSF generators, as per the original post
    Series.A = 0xf1ea5eed;
    Series.B = Value;
    Series.C = Value;
    Series.D = Value;

    uint32_t Count = 20;
    while (Count--)
    {
        RandomU64(&Series);
    }

    return Series;
}

static double RandomInRange(random_series *Series, double Min, double Max)
{
    double t = (double)RandomU64(Series) / (double)UINT64_MAX;
    double Result = (1.0 - t) * Min + t * Max;

    return Result;
}

static FILE *Open(long long unsigned PairCount, char const *Label, char const *Extension)
{
    char Temp[256];
    sprintf(Temp, "data_%llu_%s.%s", PairCount, Label, Extension);
    FILE *Result = fopen(Temp, "wb");
    if (!Result)
    {
        fprintf(stderr, "Unable to open \"%s\" for writing.\n", Temp);
    }

    return Result;
}

static double RandomDegree(random_series *Series, double Center, double Radius, double MaxAllowed)
{
    double MinVal = Center - Radius;
    if (MinVal < -MaxAllowed)
    {
        MinVal = -MaxAllowed;
    }

    double MaxVal = Center + Radius;
    if (MaxVal > MaxAllowed)
    {
        MaxVal = MaxAllowed;
    }

    double Result = RandomInRange(Series, MinVal, MaxVal);
    return Result;
}

int harvestineDistanceJsonGenerator(size_t pairCount, uint64_t seed, char *method)
{

    uint64_t clusterCountLeft = UINT64_MAX;
    double maxAllowedX = 180;
    double maxAllowedY = 90;

    double xCenter = 0;
    double yCenter = 0;
    double xRadius = maxAllowedX;
    double yRadius = maxAllowedY;

    if (strcmp(method, "cluster") == 0)
    {
        clusterCountLeft = 0;
    }
    else if (strcmp(method, "uniform") != 0)
    {

        method = "uniform";
        fprintf(stderr, "WARNING: Unrecognized method name. Using uniform by default.\n");
    }

    random_series series = Seed(seed);
    uint64_t maxPairCount = (1ULL << 34);

    if (pairCount < maxPairCount)
    {
        uint64_t clusterCountMax = 1 + (pairCount / 64);

        FILE *flexJSON = Open(pairCount, "flex", "json");
        FILE *haverAnswers = Open(pairCount, "haveranswer", "double");
        if (flexJSON && haverAnswers)
        {

            fprintf(flexJSON, "{\"pairs\": [\n");
            double sum = 0;
            double sumCoef = 1.0 / (double)pairCount;
            for (size_t i = 0; i < pairCount; i++)
            {

                if (clusterCountLeft-- == 0)
                {
                    clusterCountLeft = clusterCountMax;
                    xCenter = RandomInRange(&series, --maxAllowedX, maxAllowedX);
                    yCenter = RandomInRange(&series, --maxAllowedY, maxAllowedY);
                    xRadius = RandomInRange(&series, 0, maxAllowedX);
                    yRadius = RandomInRange(&series, 0, maxAllowedY);
                }

                double x0 = RandomDegree(&series, xCenter, xRadius, maxAllowedX);
                double y0 = RandomDegree(&series, yCenter, yRadius, maxAllowedY);
                double x1 = RandomDegree(&series, xCenter, xRadius, maxAllowedX);
                double y1 = RandomDegree(&series, yCenter, yRadius, maxAllowedY);

                double earthRadius = 6372.8;
                double haversineDistance = ReferenceHaversine(x0, x1, y0, y1, earthRadius);

                sum += sumCoef * haversineDistance;

                fprintf(flexJSON, "{\"x0\": %f, \"x1\": %f, \"y0\": %f, \"y1\": %f}", x0, x1, y0, y1);

                if (i < pairCount - 1)
                {
                    fprintf(flexJSON, ",\n");
                }

                fwrite(&haversineDistance, sizeof(haversineDistance), 1, haverAnswers);
            }

            fprintf(flexJSON, "]}");
            fwrite(&sum, sizeof(sum), 1, haverAnswers);

            fprintf(stdout, "Method: %s\n", method);
            fprintf(stdout, "Random seed: %llu\n", seed);
            fprintf(stdout, "Pair count: %llu\n", pairCount);
            fprintf(stdout, "Expected sum: %.16f\n", sum);

            if (flexJSON)
            {
                fclose(flexJSON);
            }
            if (haverAnswers)
            {
                fclose(haverAnswers);
            }
        }
    }
    else
    {
        fprintf(stderr, "To avoid accidentally generating massive files, number of pairs must be less than %llu.\n", maxPairCount);
    }
}

int main(int argc, char *argv[])
{

    char *argv1 = argv[1];
    char *argv2 = argv[2];
    char *argv3 = argv[3];

    if (argv1 && strcmp(argv1, "--help") == 0)
    {
        printf("Arguments: \n");
        printf("amount - 1\n");
        printf("seed - 2\n");
        printf("method - 3\n");

        return 0;
    }

    if (!argv1)
    {
        printf("Amount is not provided\n");
        return 0;
    }

    if (!argv2)
    {
        printf("Seed is not provided\n");
        return 0;
    }

    if (!argv3)
    {
        printf("Method is not provided\n");
        return 0;
    }

    size_t amount = strtod(argv1, NULL);
    size_t seed = strtod(argv2, NULL);
    char *method = argv3;

    harvestineDistanceJsonGenerator(amount, seed, method);
}