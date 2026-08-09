#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "../haversine-formula/haversine-formula.h"

int harvestineDistanceJsonGenerator(size_t amount, double min, double max, size_t seed)
{
    srand(seed);

    FILE *file = fopen("harvestine.json", "w");

    double averageHaversineDistance = 0;

    fprintf(file, "{\"pairs\": [");
    for (size_t i = 0; i < amount; i++)
    {
        double x0 = min + ((double)rand() / RAND_MAX) * (max - min);
        double x1 = min + ((double)rand() / RAND_MAX) * (max - min);
        double y0 = min + ((double)rand() / RAND_MAX) * (max - min);
        double y1 = min + ((double)rand() / RAND_MAX) * (max - min);

        double haversineDistance = ReferenceHaversine(x0, x1, y0, y1, 6372.8);

        averageHaversineDistance += haversineDistance;

        fprintf(file, "{\"x0\": %f, \"x1\": %f, \"y0\": %f, \"y1\": %f, haversineDistance: %f}", x0, x1, y0, y1, haversineDistance);

        if (i < amount - 1)
        {
            fprintf(file, ",");
        }
    }
    fprintf(file, "]}");

    fclose(file);

    averageHaversineDistance = averageHaversineDistance / amount;

    printf("averageHaversineDistance %f\n", averageHaversineDistance);
}

int main(int argc, char *argv[])
{

    char *argv1 = argv[1];
    char *argv2 = argv[2];
    char *argv3 = argv[3];
    char *argv4 = argv[4];

    if (argv1 && strcmp(argv1, "--help") == 0)
    {
        printf("Arguments: \n");
        printf("amount - 1\n");
        printf("min - 2\n");
        printf("max - 3\n");
        printf("seed - 4\n");

        return 0;
    }

    if (!argv1)
    {
        printf("Amount is not provided\n");
        return 0;
    }

    if (!argv2)
    {
        printf("Min is not provided\n");
        return 0;
    }

    if (!argv3)
    {
        printf("Max is not provided\n");
        return 0;
    }

    if (!argv4)
    {
        printf("Seed is not provided\n");
        return 0;
    }

    size_t amount = strtod(argv1, NULL);
    double min = strtod(argv2, NULL);
    double max = strtod(argv3, NULL); // 6372.8;
    size_t seed = strtod(argv4, NULL);

    harvestineDistanceJsonGenerator(amount, min, max, seed);
}