#include "cubiomes/generator.h"
#include "cubiomes/util.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    FILE *fp = fopen(argv[1], "r");
    if (!fp) return 2;
    char line[256], expected[128];
    int64_t seed, last = 0;
    int scale, x, y, z, total = 0, mismatch = 0;
    Generator g;
    setupGenerator(&g, MC_26_3, 0);
    fgets(line, sizeof(line), fp);
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%" SCNd64 ",%d,%d,%d,%d,%127s", &seed, &scale, &x, &y, &z, expected) != 6)
            return 2;
        if (total == 0 || seed != last) applySeed(&g, DIM_OVERWORLD, (uint64_t)seed);
        last = seed;
        int id = getBiomeAt(&g, scale, x, y, z);
        const char *actual = biome2str(MC_26_3, id);
        ++total;
        if (!actual || strcmp(expected, actual)) {
            if (mismatch++ < 20)
                printf("Mismatch seed=%" PRId64 " scale=%d (%d,%d,%d) expected=%s actual=%s\n",
                       seed, scale, x, y, z, expected, actual ? actual : "NULL");
        }
    }
    fclose(fp);
    printf("Vanilla 26.3 biome samples: %d; mismatches: %d\n", total, mismatch);
    return total != 4096 || mismatch != 0;
}
