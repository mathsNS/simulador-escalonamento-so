#include "workload.h"

#include <assert.h>
#include <stdio.h>

static void test_same_seed_same_workload(void) {
    int a[50], b[50];
    Rng rng_a, rng_b;
    size_t i;
    rng_seed(&rng_a, 42);
    rng_seed(&rng_b, 42);
    generate_arrivals(&rng_a, 50, 10.0, a);
    generate_arrivals(&rng_b, 50, 10.0, b);
    for (i = 0; i < 50; ++i)
        assert(a[i] == b[i]);
}

static void test_different_seed_different_workload(void) {
    int a[50], b[50];
    Rng rng_a, rng_b;
    size_t i;
    int differs = 0;
    rng_seed(&rng_a, 1);
    rng_seed(&rng_b, 2);
    generate_arrivals(&rng_a, 50, 10.0, a);
    generate_arrivals(&rng_b, 50, 10.0, b);
    for (i = 0; i < 50; ++i)
        if (a[i] != b[i])
            differs = 1;
    assert(differs);
}

static void test_arrivals_non_decreasing(void) {
    int a[1000];
    Rng rng;
    size_t i;
    rng_seed(&rng, 7);
    generate_arrivals(&rng, 1000, 3.0, a);
    assert(a[0] >= 0);
    for (i = 1; i < 1000; ++i)
        assert(a[i] >= a[i - 1]);
}

static void test_zero_mean_arrives_at_instant_zero(void) {
    int a[10];
    Rng rng;
    size_t i;
    rng_seed(&rng, 99);
    generate_arrivals(&rng, 10, 0.0, a);
    for (i = 0; i < 10; ++i)
        assert(a[i] == 0);
}

int main(void) {
    puts("Workload: mesma seed produz a mesma carga"); fflush(stdout);
    test_same_seed_same_workload();
    puts("Workload: seeds diferentes produzem cargas diferentes"); fflush(stdout);
    test_different_seed_different_workload();
    puts("Workload: chegadas nao decrescentes"); fflush(stdout);
    test_arrivals_non_decreasing();
    puts("Workload: media zero chega toda no instante 0"); fflush(stdout);
    test_zero_mean_arrives_at_instant_zero();
    puts("Todos os testes passaram.");
    return 0;
}
