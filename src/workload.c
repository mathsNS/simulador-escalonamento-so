#include "workload.h"

#include <math.h>

void rng_seed(Rng *rng, unsigned long long seed) {
    /* xorshift64* não pode iniciar com estado zero. */
    rng->state = seed ? seed : 0x9E3779B97F4A7C15ULL;
}

static unsigned long long xorshift64star(unsigned long long *state) {
    unsigned long long x = *state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    *state = x;
    return x * 0x2545F4914F6CDD1DULL;
}

double rng_uniform01(Rng *rng) {
    /* 53 bits de mantissa: mesma resolução usada por geradores double comuns. */
    unsigned long long bits = xorshift64star(&rng->state) >> 11;
    return (double)bits / (double)(1ULL << 53);
}

void generate_arrivals(Rng *rng, size_t count, double mean_interarrival,
                       int *arrivals_out) {
    size_t i;
    int time = 0;
    for (i = 0; i < count; ++i) {
        if (mean_interarrival > 0.0) {
            double u = rng_uniform01(rng);
            if (u <= 0.0)
                u = 1e-12; /* evita log(0) na amostragem por transformada inversa */
            time += (int)(-mean_interarrival * log(u) + 0.5);
        }
        arrivals_out[i] = time;
    }
}
